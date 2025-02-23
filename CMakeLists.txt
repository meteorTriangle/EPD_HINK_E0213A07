cmake_minimum_required(VERSION 3.18)

project(EPD C CXX)

add_library(EPD STATIC
    src/EPD.cpp
)

target_link_libraries(EPD 
    pico_stdlib
    qrcode
    hardware_spi
)

target_include_directories(EPD PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/src
)
