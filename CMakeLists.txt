cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(atio)

add_executable(atio
        atio.c
        )

target_sources(atio PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/atio.c
        )

pico_sdk_init()

target_link_libraries(atio PRIVATE pico_stdlib hardware_resets hardware_irq hardware_adc)

# enable usb output, disable uart output
pico_enable_stdio_usb(atio 1)
pico_enable_stdio_uart(atio 0)

pico_add_extra_outputs(atio)
