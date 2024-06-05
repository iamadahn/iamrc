cmake_minimum_required(VERSION 3.22)

set(SCRS
    ${CMAKE_CURRENT_LIST_DIR}/display_controller.c
)

add_library(display_controller INTERFACE)
target_sources(display_controller INTERFACE ${SCRS})
target_include_directories(display_controller INTERFACE
    root
)
target_compile_definitions(display_controller INTERFACE root)
target_link_libraries(display_controller INTERFACE
    root
)
