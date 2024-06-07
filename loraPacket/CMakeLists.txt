add_library(loraPacket INTERFACE)

target_include_directories(loraPacket INTERFACE ./include)

target_sources(loraPacket
        INTERFACE
        loraPacket.cpp
)