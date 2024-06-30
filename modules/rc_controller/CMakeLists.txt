cmake_minimum_required(VERSION 3.22)

set(SCRS
    ${CMAKE_CURRENT_LIST_DIR}/rc_controller.c
)

add_library(rc_controller_module INTERFACE)
target_sources(rc_controller_module INTERFACE ${SCRS})
target_include_directories(rc_controller_module INTERFACE 
    root
)
target_compile_definitions(rc_controller_module INTERFACE root)
target_link_libraries(rc_controller_module INTERFACE
    root
)
