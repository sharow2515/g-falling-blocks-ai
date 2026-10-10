add_library(cli STATIC
    arg_parser.cpp
    common_options.cpp
)

target_link_libraries(cli PUBLIC ai)
target_include_directories(cli PUBLIC ${CMAKE_SOURCE_DIR}/src)
