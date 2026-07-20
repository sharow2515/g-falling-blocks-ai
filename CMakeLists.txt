cmake_minimum_required(VERSION 3.21)
project(FallingBlocks CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

add_subdirectory(src/core)

add_executable(FallingBlocks
    src/main.cpp
    src/app/input_controller.cpp
    src/app/renderer.cpp
    src/app/application.cpp
)
target_link_libraries(FallingBlocks PRIVATE core SFML::Graphics SFML::Window SFML::System)
target_include_directories(FallingBlocks PRIVATE src)
