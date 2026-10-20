add_library(ai STATIC
    factory.cpp
    common/board_features.cpp
    common/occupancy_grid.cpp
    common/reachable_search.cpp
    common/move_execution.cpp
    common/executable_plan.cpp
    common/spawn_state.cpp
    simple/placement.cpp
    simple/evaluator.cpp
    simple/simple_player.cpp
    reachability/evaluator.cpp
    reachability/reachability_player.cpp
    beam/evaluator.cpp
    beam/beam_search_player.cpp
    beam_tuned/evaluator.cpp
    beam_tuned/beam_search_player.cpp
    beam_unique/evaluator.cpp
    beam_unique/beam_search_player.cpp
)

target_link_libraries(ai PUBLIC core OpenMP::OpenMP_CXX)
target_include_directories(ai PUBLIC ${CMAKE_SOURCE_DIR}/src)
