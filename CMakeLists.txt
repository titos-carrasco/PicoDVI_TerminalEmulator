cmake_minimum_required(VERSION 3.12)

# pico-sdk
set(PICO_PLATFORM rp2040)
set(PICO_BOARD pico)

# PicoDVI
set(PICO_DVI_PATH $ENV{PICO_DVI_PATH})
set(PICO_COPY_TO_RAM 1)

# TinyUSB
set(FAMILY rp2040)
set(BOARD raspberry_pi_pico)
set(PICO_TINYUSB_HOST 1)

# para ayudar a localizar el SDK
include(pico_sdk_import.cmake)

# define mi proyecto
project(TerminalEmulator C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
add_compile_options(-Wall)

# inicializa Pico SDK
pico_sdk_init()

# crea el target con mis fuentes
add_executable(TerminalEmulator
	./src/main.cpp
    ./src/DVIDisplay.cpp
    ./src/HIDKeyboard.cpp
    ./src/vt100.cpp
    ./src/automata.cpp
)

# agrega otras fuentes
target_sources(TerminalEmulator PRIVATE
    ${PICO_DVI_PATH}/software/libdvi/dvi.c
    ${PICO_DVI_PATH}/software/libdvi/dvi_timing.c
    ${PICO_DVI_PATH}/software/libdvi/dvi_serialiser.c
    ${PICO_DVI_PATH}/software/libdvi/tmds_encode.c
    ${PICO_DVI_PATH}/software/libdvi/tmds_encode.S
)

# agrega defines
set(DVI_DEFAULT_SERIAL_CONFIG waveshare_rp2040_pizero)
target_compile_definitions(TerminalEmulator PRIVATE
	DVI_DEFAULT_SERIAL_CONFIG=${DVI_DEFAULT_SERIAL_CONFIG}
	DVI_VERTICAL_REPEAT=1
	DVI_N_TMDS_BUFFERS=3
	DVI_MONOCHROME_TMDS=1
    RCR_DEBUG=1
)

# agrega directorios para los include
target_include_directories(TerminalEmulator PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${PICO_SDK_PATH}/src/rp2_common/hardware/include
    ${PICO_SDK_PATH}/src/rp2_common/pico_stdlib/include
    ${PICO_DVI_PATH}/software/include
    ${PICO_DVI_PATH}/software/libdvi
    ${PICO_SDK_PATH}/lib/tinyusb/src
)

# agrega las librerias requeridas
target_link_libraries(TerminalEmulator
    pico_stdlib
    pico_multicore
    pico_util
    hardware_vreg
    hardware_clocks
    hardware_dma
    hardware_pio
    hardware_pwm
    hardware_interp
    tinyusb_host
)

# compila algunos asm llevandolos a .h
pico_generate_pio_header(TerminalEmulator
    ${PICO_DVI_PATH}/software/libdvi/tmds_encode_1bpp.pio
    ${PICO_DVI_PATH}/software/libdvi/dvi_serialiser.pio
)

# habilita/deshabilita salida estandar por USB
pico_enable_stdio_usb(TerminalEmulator 0)

# habilita/deshabilita la salida estandar por UART0
pico_enable_stdio_uart(TerminalEmulator 0)

# archivos finales UF2, bin, elf
pico_add_extra_outputs(TerminalEmulator)
