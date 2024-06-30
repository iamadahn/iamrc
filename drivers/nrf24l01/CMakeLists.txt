cmake_minimum_required(VERSION 3.22)

set(SCRS
    ${CMAKE_CURRENT_LIST_DIR}/nrf24l01.c
)

add_library(nrf24l01_driver INTERFACE)
target_sources(nrf24l01_driver INTERFACE ${SCRS})
target_include_directories(nrf24l01_driver  INTERFACE 
    root
)
target_compile_definitions(nrf24l01_driver  INTERFACE root)
target_link_libraries(nrf24l01_driver  INTERFACE
    root
)
