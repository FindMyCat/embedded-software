cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(homestation_esp32)

zephyr_include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(src/common)
add_subdirectory(src/modules/ble)
# Not built: TRIGGER_CHAN is declared ZBUS_OBSERVERS_EMPTY, so this thread
# publishes to nothing. Restore it along with whatever subscribes.
# add_subdirectory(src/modules/trigger)
add_subdirectory(src/modules/lora)
add_subdirectory(src/modules/mqtt)
add_subdirectory(src/modules/wifi)
add_subdirectory(src/modules/error)

target_sources(app PRIVATE src/main.c)
