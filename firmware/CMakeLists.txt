cmake_minimum_required(VERSION 3.20)

# pico_sdk_import.cmake must exist before project() is called.
# Copy it from your pico-sdk: cp $PICO_SDK_PATH/external/pico_sdk_import.cmake .
include(pico_sdk_import.cmake)

project(lpd8_if C CXX ASM)
set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(lpd8_if
    src/main.c
    src/usb_midi_host.c
    src/midi_handler.c
    src/mcp4728.c
    src/gate_driver.c
)

# Project root must be on the include path so TinyUSB can find tusb_config.h
# (TinyUSB uses #include "tusb_config.h" — quoted, so it searches -I paths).
target_include_directories(lpd8_if PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# pico_stdlib       — GPIO, UART, timers
# hardware_i2c      — I2C peripheral (MCP4728)
# tinyusb_host      — TinyUSB host stack (native RP2040/RP2350 USB controller)
# tinyusb_board     — BSP glue required by TinyUSB
target_link_libraries(lpd8_if
    pico_stdlib
    hardware_i2c
    tinyusb_host
    tinyusb_board
)

# UART stdio for debug output (GP0/GP1). Disable for final panel build.
pico_enable_stdio_uart(lpd8_if 1)
pico_enable_stdio_usb(lpd8_if 0)

# Generate .uf2, .bin, .map, .dis
pico_add_extra_outputs(lpd8_if)

target_compile_options(lpd8_if PRIVATE
    -Wall
    -Wextra
    -Wno-unused-parameter   # TinyUSB callbacks have unused params by design
)
