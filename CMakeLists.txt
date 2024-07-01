cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)
project(cansat C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

pico_sdk_init()

add_subdirectory(SX1276)
add_subdirectory(BME280)
add_subdirectory(ICM20948)
add_subdirectory(loraPacket)
add_subdirectory(INTERRUPT)
add_subdirectory(GPS)
add_subdirectory(PCF8574)
#add_subdirectory(FileSystem)
#add_subdirectory(SDCard)
add_subdirectory(FatFs_SPI build)

add_executable(cansat
        main.cpp
)

pico_enable_stdio_usb(cansat 0)
pico_enable_stdio_uart(cansat 1)

target_link_libraries(cansat
        pico_stdlib
        pico_multicore
#        pico_mutex
        hardware_spi
        hardware_dma
        hardware_watchdog
        SX1276
        loraPacket
        BME280
        ICM20948
        INTERRUPT
        GPS
        PCF8574
        FatFs_SPI

)

pico_add_extra_outputs(cansat)