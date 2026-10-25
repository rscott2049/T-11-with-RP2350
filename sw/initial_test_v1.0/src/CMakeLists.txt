# Leaf CMakeLists.txt
add_executable(initial_test
	initial_test.c
	rl02.c
	rl02_storage.c
	load_lda.c
	hw_config.c
	)

target_link_libraries(initial_test
	pico_stdlib
	pico_multicore
	hardware_pio
	hardware_pwm
	hardware_dma
	no-OS-FatFS-SD-SDIO-SPI-RPi-Pico
        )	

pico_add_extra_outputs(initial_test)

# Configure outputs
pico_enable_stdio_usb(initial_test 0)
pico_enable_stdio_uart(initial_test 1)

# Use J6 as serial port
#target_compile_definitions(initial_test PRIVATE
#  PICO_DEFAULT_UART=1
#  PICO_DEFAULT_UART_TX_PIN=8
#  PICO_DEFAULT_UART_RX_PIN=9
#)

# Color emu rev 1.1: Use J7 as serial port 
#target_compile_definitions(initial_test PRIVATE
#  PICO_DEFAULT_UART=1
#  PICO_DEFAULT_UART_TX_PIN=38
#  PICO_DEFAULT_UART_RX_PIN=39
#)

# Color emu rev 1.2: Use J5 as serial port 
#target_compile_definitions(initial_test PRIVATE
#  PICO_DEFAULT_UART=0
#  PICO_DEFAULT_UART_TX_PIN=16
#  PICO_DEFAULT_UART_RX_PIN=17
#)

# pico2-t11: Use J5 as serial port 
target_compile_definitions(initial_test PRIVATE
#  PICO_PIO_USE_GPIO_BASE=1
  PICO_DEFAULT_UART=0
  PICO_DEFAULT_UART_TX_PIN=0
  PICO_DEFAULT_UART_RX_PIN=1
)

pico_generate_pio_header(initial_test ${CMAKE_CURRENT_LIST_DIR}/t11_bus.pio)
pico_generate_pio_header(initial_test ${CMAKE_CURRENT_LIST_DIR}/la.pio)

# run from RAM
pico_set_binary_type(initial_test no_flash)

