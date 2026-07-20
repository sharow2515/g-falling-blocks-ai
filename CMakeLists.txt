cmake_minimum_required(VERSION 3.21)
project(FallingBlocks CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

add_executable(FallingBlocks src/main.cpp)
target_link_libraries(FallingBlocks PRIVATE SFML::Graphics SFML::Window SFML::System)