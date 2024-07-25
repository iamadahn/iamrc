cmake_minimum_required(VERSION 3.22)

set(SCRS
    ${CMAKE_CURRENT_LIST_DIR}/usbd_controller.c
)

add_library(usbd_controller_module INTERFACE)
target_sources(usbd_controller_module INTERFACE ${SCRS})
target_include_directories(usbd_controller_module INTERFACE 
    root
)
target_compile_definitions(usbd_controller_module INTERFACE root)
target_link_libraries(usbd_controller_module INTERFACE
    root
)
