cmake_minimum_required(VERSION 3.22)

set(SCRS
    ${CMAKE_CURRENT_LIST_DIR}/analog_stick_controller.c
)

add_library(analog_stick_controller INTERFACE)
target_sources(analog_stick_controller INTERFACE ${SCRS})
target_include_directories(analog_stick_controller INTERFACE 
    root
)
target_compile_definitions(analog_stick_controller INTERFACE root)
target_link_libraries(analog_stick_controller INTERFACE
    root
)
