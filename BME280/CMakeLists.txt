add_library(BME280 INTERFACE)

target_include_directories(BME280 INTERFACE ./include)

target_sources(BME280
        INTERFACE
        BME280.cpp
)

target_link_libraries(BME280
        INTERFACE
        pico_stdlib
        hardware_i2c
)
