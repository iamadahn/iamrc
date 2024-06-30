cmake_minimum_required(VERSION 3.22)

set(SCRS
)

add_library(analog_stick_driver INTERFACE)
target_sources(analog_stick_driver INTERFACE ${SCRS})
target_include_directories(analog_stick_driver INTERFACE 
    root
)
target_compile_definitions(analog_stick_driver INTERFACE root)
target_link_libraries(analog_stick_driver INTERFACE
    root
)
