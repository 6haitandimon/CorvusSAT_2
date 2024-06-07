add_library(SX1276 INTERFACE)

target_include_directories(SX1276 INTERFACE ./include)

target_sources(SX1276
        INTERFACE
        sx1276.cpp
)

target_link_libraries(SX1276
        INTERFACE
        pico_stdlib
        loraPacket
)
