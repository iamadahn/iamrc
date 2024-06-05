cmake_minimum_required(VERSION 3.22)

set(SCRS
    ${CMAKE_CURRENT_LIST_DIR}/ssd1306.c
    ${CMAKE_CURRENT_LIST_DIR}/fonts.c
)

add_library(ssd1306 INTERFACE)
target_sources(ssd1306 INTERFACE ${SCRS})
target_include_directories(ssd1306 INTERFACE 
    root
)
target_compile_definitions(ssd1306 INTERFACE root)
target_link_libraries(ssd1306 INTERFACE
    root
)
