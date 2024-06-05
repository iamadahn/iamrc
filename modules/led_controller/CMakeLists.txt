cmake_minimum_required(VERSION 3.22)

set(SCRS
    ${CMAKE_CURRENT_LIST_DIR}/led_controller.c
)

add_library(led_controller INTERFACE)
target_sources(led_controller INTERFACE ${SCRS})
target_include_directories(led_controller INTERFACE 
    root
)
target_compile_definitions(led_controller INTERFACE root)
target_link_libraries(led_controller INTERFACE
    root
)
