cmake_minimum_required(VERSION 3.22)

set(SCRS
    ${CMAKE_CURRENT_LIST_DIR}/led.c
)

add_library(led_driver INTERFACE)
target_sources(led_driver INTERFACE ${SCRS})
target_include_directories(led_driver INTERFACE 
    root
)
target_compile_definitions(led_driver INTERFACE root)
target_link_libraries(led_driver INTERFACE
    root
)
