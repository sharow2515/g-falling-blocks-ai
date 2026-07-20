add_library(ai STATIC
    placement.cpp
    evaluator.cpp
    heuristic_ai.cpp
)

target_link_libraries(ai PUBLIC core)
target_include_directories(ai PUBLIC ${CMAKE_SOURCE_DIR}/src)
