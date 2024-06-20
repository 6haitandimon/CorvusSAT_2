add_library(INTERRUPT INTERFACE)

target_include_directories(INTERRUPT INTERFACE ./include)

target_sources(INTERRUPT
        INTERFACE
        interrupt.cpp
)

target_link_libraries(INTERRUPT
        INTERFACE
        pico_stdlib
        SX1276
)