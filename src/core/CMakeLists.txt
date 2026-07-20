find_package(nlohmann_json CONFIG REQUIRED)

add_library(core STATIC
    mino.cpp
    mino_loader.cpp
    wall_kick.cpp
    board.cpp
    active_piece.cpp
    bag_generator.cpp
    game.cpp
)

target_link_libraries(core PUBLIC nlohmann_json::nlohmann_json)
target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/src)
