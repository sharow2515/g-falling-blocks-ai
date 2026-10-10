cmake_minimum_required(VERSION 3.21)
project(FallingBlocks CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "" FORCE)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -mpopcnt")
endif()

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)
find_package(OpenMP REQUIRED)

add_subdirectory(src/core)
add_subdirectory(src/ai)

add_executable(FallingBlocks
    src/main.cpp
    src/app/input_controller.cpp
    src/app/renderer.cpp
    src/app/application_base.cpp
    src/app/human_play_application.cpp
    src/app/ai_visualize_application.cpp
)
target_link_libraries(FallingBlocks PRIVATE core ai SFML::Graphics SFML::Window SFML::System)
target_include_directories(FallingBlocks PRIVATE src)

add_executable(Benchmark
    src/benchmark/benchmark_main.cpp
    src/benchmark/runner.cpp
)
target_link_libraries(Benchmark PRIVATE core ai)
target_include_directories(Benchmark PRIVATE src)
