cmake_minimum_required(VERSION 3.17)
project(Paint)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp)
add_executable(Paint ${SOURCE_FILES})


set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
find_package(SFML 2 REQUIRED system graphics window network audio)

include_directories(${SFML_INCLUDE_DIR})
target_link_libraries(Paint ${SFML_LIBRARIES})
