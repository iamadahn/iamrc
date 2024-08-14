cmake_minimum_required(VERSION 3.22)

set(SCRS
    ${CMAKE_CURRENT_LIST_DIR}/logger.c
)

add_library(logger_driver INTERFACE)
target_sources(logger_driver INTERFACE ${SCRS})
target_include_directories(logger_driver INTERFACE 
    root
)
target_compile_definitions(logger_driver INTERFACE root)
target_link_libraries(logger_driver INTERFACE
    root
)
