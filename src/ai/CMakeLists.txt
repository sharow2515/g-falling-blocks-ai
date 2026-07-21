add_library(ai STATIC
    heuristic/placement.cpp
    heuristic/evaluator.cpp
    heuristic/heuristic_player.cpp
)

target_link_libraries(ai PUBLIC core)
target_include_directories(ai PUBLIC ${CMAKE_SOURCE_DIR}/src)
