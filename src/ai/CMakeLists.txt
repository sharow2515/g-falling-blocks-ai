add_library(ai STATIC
    common/board_features.cpp
    common/reachable_search.cpp
    common/move_execution.cpp
    common/path_conversion.cpp
    heuristic/placement.cpp
    heuristic/evaluator.cpp
    heuristic/heuristic_player.cpp
    reachability/evaluator.cpp
    reachability/reachability_player.cpp
    beam/evaluator.cpp
    beam/beam_search_player.cpp
)

target_link_libraries(ai PUBLIC core)
target_include_directories(ai PUBLIC ${CMAKE_SOURCE_DIR}/src)
