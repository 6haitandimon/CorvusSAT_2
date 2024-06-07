add_library(ICM20948 INTERFACE)

target_include_directories(ICM20948 INTERFACE ./include)

target_sources(ICM20948
        INTERFACE
        ICM20948.cpp
)

target_link_libraries(ICM20948
        INTERFACE
        pico_stdlib
        hardware_i2c
)

