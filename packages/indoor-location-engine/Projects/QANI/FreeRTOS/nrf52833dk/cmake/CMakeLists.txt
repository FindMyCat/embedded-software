cmake_minimum_required(VERSION 3.20)

# Force the toolchain BEFORE project() so cmake doesn't run host compiler probes.
if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE
        "${CMAKE_CURRENT_LIST_DIR}/../../cmake/arm-none-eabi.cmake"
        CACHE FILEPATH "" FORCE)
endif()

project(nRF52833DK-QANI-FreeRTOS C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PROJECT_NAME            "nRF52833DK-QANI-FreeRTOS")
set(BOARD                   "nRF52833DK")
get_filename_component(REPO_ROOT             "${CMAKE_CURRENT_LIST_DIR}/../../../../.." ABSOLUTE)
get_filename_component(PROJ_FREERTOS_DIR     "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
get_filename_component(PROJECT_DEFINITION_DIR "${CMAKE_CURRENT_LIST_DIR}/../ProjectDefinition" ABSOLUTE)
set(LINKER_SCRIPT           "${PROJ_FREERTOS_DIR}/qani_nrf52833_s140.ld")

include(${PROJ_FREERTOS_DIR}/cmake/qani_common.cmake)
