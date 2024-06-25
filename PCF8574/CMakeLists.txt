add_library(PCF8574 INTERFACE)

target_include_directories(PCF8574 INTERFACE ./include)

target_sources(PCF8574
        INTERFACE
        PCF8574.cpp
)
target_link_libraries(PCF8574
        INTERFACE
        pico_stdlib
        hardware_i2c
)
