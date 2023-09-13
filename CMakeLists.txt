cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)

project(uart_module C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

pico_sdk_init()


add_executable(uart_module
        uart_module.c
        )

# pull in common dependencies
target_link_libraries(uart_module
pico_stdlib
pico_cyw43_arch_none     # we need Wifi to access the GPIO, but we don't need anything else
)

# create map/bin/hex file etc.
pico_add_extra_outputs(uart_module)

pico_enable_stdio_usb(uart_module 1)
pico_enable_stdio_uart(uart_module 0)
