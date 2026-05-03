cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(homestation_esp32)

add_subdirectory(src/common)
add_subdirectory(src/modules/trigger)
add_subdirectory(src/modules/lora)
add_subdirectory(src/modules/error)

target_sources(app PRIVATE src/main.c)
