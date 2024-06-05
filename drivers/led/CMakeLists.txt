cmake_minimum_required(VERSION 3.22)

set(SCRS
    ${CMAKE_CURRENT_LIST_DIR}/led.c
)

add_library(led INTERFACE)
target_sources(led INTERFACE ${SCRS})
target_include_directories(led INTERFACE 
    root
)
target_compile_definitions(led INTERFACE root)
target_link_libraries(led INTERFACE
    root
)
