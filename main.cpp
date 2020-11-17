#include <SFML/Graphics.hpp>
#include <stack>
#include <iostream>

using namespace std;
using namespace sf;

const int WIDTH = 750;
const int HEIGHT = 750;

const int CANVAS_WIDTH = WIDTH;
const int CANVAS_HEIGHT = HEIGHT - 50;

const int PALETTE_SIZE = 40;
const int RAINBOW_MAX = 500;

bool inCanvas(int x, int y) {
    return x >= 0 && y >= 0 && x < CANVAS_WIDTH && y < CANVAS_HEIGHT;
}

bool inRadius(int x1, int x2, int y1, int y2, int radius) {
    int x = x1 - x2;
    int y = y1 - y2;

    int distance = x * x + y * y;

    return distance <= (radius * radius);
}

class Change {
public:
    int x;
    int y;
    int r;
    int g;
    int b;

    Change(int x, int y, int r, int g, int b) {
        this->x = x;
        this->y = y;
        this->r = r;
        this->g = g;
        this->b = b;
    }
};

class Stroke {
public:
    vector<Change> changes;

    void undo(Image *image);

    void redo(Image *image);

    void draw(Image *image) {
        for (Change change : changes) {
            int x = change.x;
            int y = change.y;

            image->setPixel(x, y, Color(change.r, change.g, change.b));
        }
    }
};

stack<Stroke> undoHistory;
stack<Stroke> redoHistory;

void Stroke::undo(Image *image) {
    Stroke stroke;

    for (Change change : changes) {
        Color old = image->getPixel(change.x, change.y);

        if (old != Color(change.r, change.g, change.b)) {
            stroke.changes.push_back(Change(change.x, change.y, old.r, old.g, old.b));
        }
    }

    redoHistory.push(stroke);

    this->draw(image);
}

void Stroke::redo(Image *image) {
    Stroke stroke;

    for (Change change : changes) {
        Color old = image->getPixel(change.x, change.y);

        if (old != Color(change.r, change.g, change.b)) {
            stroke.changes.push_back(Change(change.x, change.y, old.r, old.g, old.b));
        }
    }

    undoHistory.push(stroke);

    this->draw(image);
}


enum BrushType {
    CIRCLE,
    SQUARE
};

class Brush {
public:
    Color color;
    BrushType type;
    int radius;

    Brush(Color color, int radius) {
        this->color = color;
        this->radius = radius;
        this->type = BrushType::CIRCLE;
    }

    void draw(Image *image, int x, int y) {
        if (!inCanvas(x, y)) return;

        Stroke stroke;

        for (int i = -radius; i <= radius; i++) {
            for (int k = -radius; k <= radius; k++) {
                int xi = x + i;
                int yk = y + k;

                if (!inCanvas(xi, yk)) continue;

                if (this->type == BrushType::CIRCLE && !inRadius(xi, x, yk, y, radius)) continue;

                Color old = image->getPixel(xi, yk);

                if (old != color) {
                    stroke.changes.push_back(Change(xi, yk, old.r, old.g, old.b));
                    image->setPixel(xi, yk, color);
                }
            }
        }

        if (!stroke.changes.empty()) undoHistory.push(stroke);
    }
};

class ColorSprite {
public:
    Color color;
    Texture texture;
    Image image;
    Sprite *sprite;

    explicit ColorSprite(Color color) {
        this->color = color;
        this->sprite = new Sprite;

        image.create(PALETTE_SIZE, PALETTE_SIZE);
        texture.create(PALETTE_SIZE, PALETTE_SIZE);

        for (int x = 0; x < PALETTE_SIZE; x++) {
            for (int y = 0; y < PALETTE_SIZE; y++) {
                Color c = (x == 0 || y == 0 || x == PALETTE_SIZE - 1 || y == PALETTE_SIZE - 1) ?
                          (color == Color::Black ? Color::White : Color::Black) : color;
                image.setPixel(x, y, c);
            }
        }

        texture.update(image);
        sprite->setTexture(texture);
    }

    void draw(RenderWindow *window, float x, float y) {
        sprite->setPosition(x, y);

        window->draw(*sprite);
    }
};

vector<ColorSprite> palette;

void loadPalette() {
    vector<Color> colors;

    colors.push_back(Color::Red);
    colors.push_back(Color(255, 165, 0)); // orange
    colors.push_back(Color::Yellow);
    colors.push_back(Color::Green);
    colors.push_back(Color::Blue);
    colors.push_back(Color::Cyan);
    colors.push_back(Color(255, 192, 203)); // pink
    colors.push_back(Color::Magenta);
    colors.push_back(Color(128, 0, 128)); // purple
    colors.push_back(Color::Black);
    colors.push_back(Color::White);

    for (Color color : colors) {
        palette.push_back(ColorSprite(color));
    }
}

void drawPalette(RenderWindow *window, Brush *brush, Brush *brush1) {
    int w = 10;

    ColorSprite(brush->color).draw(window, w, CANVAS_HEIGHT + 5);
    w += PALETTE_SIZE + 10;

    ColorSprite(brush1->color).draw(window, w, CANVAS_HEIGHT + 5);
    w = CANVAS_WIDTH - PALETTE_SIZE - 10;

    for (ColorSprite color : palette) {
        if (color.color == brush->color || color.color == brush1->color) {
            color.sprite->setPosition(-1, -1);
            continue;
        }

        color.draw(window, w, CANVAS_HEIGHT + 5);

        w -= (PALETTE_SIZE + 10);
    }
}

void fill(Image *image, Color color) {
    Stroke stroke;

    for (int x = 0; x < image->getSize().x; x++) {
        for (int y = 0; y < image->getSize().y; y++) {
            Color old = image->getPixel(x, y);

            image->setPixel(x, y, color);
            stroke.changes.push_back(Change(x, y, old.r, old.g, old.b));
        }
    }

    undoHistory.push(stroke);
}

void undo(Image *image) {
    if (undoHistory.empty()) return;

    Stroke stroke = undoHistory.top();
    stroke.undo(image);

    undoHistory.pop();
}

void redo(Image *image) {
    if (redoHistory.empty()) return;

    Stroke stroke = redoHistory.top();
    stroke.redo(image);

    redoHistory.pop();
}

Color rainbow(double ratio) {
    cout << "ratio " << ratio << endl;
    int normalized = int(ratio * 256 * 6);
    cout << "normalized " << normalized << endl;

    int x = normalized % 256;

    cout << "x " << x << endl;

    int red = 0, green = 0, blue = 0;
    switch(normalized / 256)
    {
        case 0: red = 255; green = x; blue = 0;       break;//red
        case 1: red = 255 - x; green = 255; blue = 0;       break;//yellow
        case 2: red = 0; green = 255; blue = x;       break;//green
        case 3: red = 0; green = 255 - x; blue = 255;     break;//cyan
        case 4: red = x; green = 0; blue = 255;     break;//blue
        case 5: red = 255; green = 0; blue = 255 - x; break;//magenta
    }

    cout << "r " << red << " g " << green << " b " << blue << endl;
    return Color(red, green, blue);
}

int main() {
    RenderWindow window(VideoMode(WIDTH, HEIGHT), "Paint",
                        Style::Titlebar | Style::Close);
    Texture texture;
    Image image;
    Sprite sprite;
    Brush leftBrush(Color::Black, 10);
    Brush rightBrush(Color::White, 10);

    bool rainbowBrush = false;
    bool focused = true;
    double ratio = 0;

    texture.create(CANVAS_WIDTH, CANVAS_HEIGHT);
    image.create(CANVAS_WIDTH, CANVAS_HEIGHT, Color::White);
    sprite.setTexture(texture);

    Clock clock;

    srand(time(nullptr));
    loadPalette();

    while (window.isOpen()) {
        Event event;

        while (window.pollEvent(event)) {
            if (event.type == Event::Closed) {
                window.close();
            } else if (event.type == Event::GainedFocus) {
                focused = true;
            } else if (event.type == Event::LostFocus) {
                focused = false;
            }
            else if (event.type == Event::MouseButtonPressed) {
                if (event.mouseButton.button == Mouse::Button::Left) {
                    int x = event.mouseButton.x;
                    int y = event.mouseButton.y;

                    for (ColorSprite color : palette) {
                        if (color.sprite->getGlobalBounds().contains(x, y)) {
                            leftBrush.color = color.color;
                            break;
                        }
                    }
                } else if (event.mouseButton.button == Mouse::Button::Right) {
                    int x = event.mouseButton.x;
                    int y = event.mouseButton.y;

                    for (ColorSprite color : palette) {
                        if (color.sprite->getGlobalBounds().contains(x, y)) {
                            rightBrush.color = color.color;
                            break;
                        }
                    }
                } else if (event.mouseButton.button == Mouse::Button::Middle) {
                    fill(&image, leftBrush.color);
                }
            } else if (event.type == Event::MouseWheelScrolled) {
                bool up = event.mouseWheelScroll.delta > 0;

                int radius = leftBrush.radius + (up ? 1 : -1);
                if (radius < 1) radius = 1;

                leftBrush.radius = radius;
                rightBrush.radius = radius;
            } else if (event.type == Event::KeyReleased) {
                if (event.key.code == Keyboard::C) {
                    fill(&image, Color::White);
                } else if (event.key.code == Keyboard::B) {
                    BrushType type = leftBrush.type == BrushType::CIRCLE ? BrushType::SQUARE : BrushType::CIRCLE;

                    leftBrush.type = type;
                    rightBrush.type = type;
                } else if (event.key.code == Keyboard::I) {
                    rainbowBrush = !rainbowBrush;
                }
            }
        }

        if (focused) {
            if (Mouse::isButtonPressed(Mouse::Button::Left)) {
                leftBrush.draw(&image, Mouse::getPosition(window).x, Mouse::getPosition(window).y);
            } else if (Mouse::isButtonPressed(Mouse::Button::Right)) {
                rightBrush.draw(&image, Mouse::getPosition(window).x, Mouse::getPosition(window).y);
            }

            if (Keyboard::isKeyPressed(Keyboard::LControl)) {
                if (Keyboard::isKeyPressed(Keyboard::Z)) {
                    undo(&image);
                } else if (Keyboard::isKeyPressed(Keyboard::R)) {
                    redo(&image);
                }
            }
        }

        if (rainbowBrush) {
            Color color = rainbow(ratio / RAINBOW_MAX);
            leftBrush.color = color;

            ratio++;

            if (ratio >= RAINBOW_MAX) ratio = 0;
        }

        window.clear(Color::White);

        texture.update(image);
        window.draw(sprite);

        drawPalette(&window, &leftBrush, &rightBrush);

        window.display();
    }


    return 0;
}