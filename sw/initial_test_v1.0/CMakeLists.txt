# Usage:
# Configure
#    cmake -B build -S .
# Build .elf, .uf2 etc.
#    make -C build/src
# Results wiil be in build/src

# Top level cmakefile
cmake_minimum_required(VERSION 3.12)

# Pass in board type, via: cmake -DPICO_BOARD=board_type
# Some board types are: pico, pico2, pimoroni_pico_plus2_rp2350
#if(PICO_BOARD)
#    set(PICO_BOARD ${PICO_BOARD})
#else()
#    set(PICO_BOARD "pico")
#endif()

#message(STATUS "PICO_BOARD: ${PICO_BOARD}")

#set(PICO_BOARD "pico")
#set(PICO_BOARD "pico2")
set(PICO_BOARD "pimoroni_pico_plus2_rp2350")

# For color_emu
#set(PICO_PLATFORM rp2350)
#set(PICO_NUM_GPIOS 48)

# Use fastest available clock
add_definitions(-DPICO_USE_FASTEST_SUPPORTED_CLOCK=1)

include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(initial_test C CXX ASM)
#add_compile_options(-O0)

pico_sdk_init(C CXX ASM)

# libraries
add_subdirectory(./no-OS-FatFS-SD-SDIO-SPI-RPi-Pico/src)

# source goes here
add_subdirectory(./src)

