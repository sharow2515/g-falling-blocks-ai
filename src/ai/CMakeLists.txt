add_library(ai STATIC
    heuristic/placement.cpp
    heuristic/evaluator.cpp
    heuristic/heuristic_player.cpp
    reachability/reachable_placement.cpp
    reachability/evaluator.cpp
    reachability/reachability_player.cpp
    beam/reachable_placement.cpp
    beam/evaluator.cpp
    beam/beam_search_player.cpp
)

target_link_libraries(ai PUBLIC core)
target_include_directories(ai PUBLIC ${CMAKE_SOURCE_DIR}/src)
