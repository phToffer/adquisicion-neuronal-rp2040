cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()

set(sdkVersion 2.3.1)
set(toolchainVersion 15_2_Rel1)
set(picotoolVersion 2.3.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)

if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()

set(PICO_BOARD pico CACHE STRING "Board type")

include(pico_sdk_import.cmake)

project(cucaracha C CXX ASM)

pico_sdk_init()

add_executable(cucaracha
        cucaracha.cpp
)

pico_set_program_name(cucaracha "cucaracha")
pico_set_program_version(cucaracha "0.1")

pico_enable_stdio_uart(cucaracha 0)
pico_enable_stdio_usb(cucaracha 1)

target_link_libraries(cucaracha
        pico_stdlib
        hardware_adc
        hardware_dma
        hardware_irq
)

target_include_directories(cucaracha PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

pico_add_extra_outputs(cucaracha)