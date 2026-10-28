# qani_common.cmake
# Shared build definition for QANI/FreeRTOS targets. Mirrors common.mk.
#
# Per-board CMakeLists.txt must set before include():
#   PROJECT_NAME            e.g. nRF52833DK-QANI-FreeRTOS
#   BOARD                   e.g. nRF52833DK
#   REPO_ROOT               absolute path to packages/indoor-location-engine/
#   PROJ_FREERTOS_DIR       absolute path to Projects/QANI/FreeRTOS/
#   PROJECT_DEFINITION_DIR  absolute path to per-board ProjectDefinition/
#   LINKER_SCRIPT           absolute path to .ld file

set(SDK_ROOT       "${REPO_ROOT}/SDK_BSP/Nordic/NORDIC_SDK_17_1_0")
set(SRC_ROOT       "${REPO_ROOT}/Src")
set(LIB_ROOT       "${REPO_ROOT}/Libs")
set(SOFTDEVICE_HEX "${SDK_ROOT}/components/softdevice/s140/hex/s140_nrf52_7.2.0_softdevice.hex")

set(SDK_COMP "${SDK_ROOT}/components")
set(SDK_LIB  "${SDK_COMP}/libraries")
set(SDK_BLE  "${SDK_COMP}/ble")
set(SDK_SVC  "${SDK_BLE}/ble_services")
set(SDK_SD   "${SDK_COMP}/softdevice")
set(SDK_EXT  "${SDK_ROOT}/external")
set(SDK_FRT  "${SDK_EXT}/freertos")
set(SDK_MOD  "${SDK_ROOT}/modules/nrfx")
set(SDK_INT  "${SDK_ROOT}/integration/nrfx")
set(SDK_MBED "${SDK_EXT}/mbedtls")
set(QANI_APP "${SRC_ROOT}/Apps/Src")
set(QANI_BLE "${SRC_ROOT}/Comm/Src/BLE")
set(QANI_BUZ "${SRC_ROOT}/Comm/Src/Buzzer")
set(QANI_I2C "${SRC_ROOT}/Comm/Src/I2C")
set(QANI_CFG "${SRC_ROOT}/Comm/Src/Config")
set(QANI_HAL "${SRC_ROOT}/HAL/Src/nrfx")
set(QANI_BRD "${SRC_ROOT}/Boards/Src")

# ------------------------------------------------------------------
# Sources
# ------------------------------------------------------------------
set(SRC_FILES
    ${SDK_MOD}/mdk/gcc_startup_nrf52833.S
    ${SDK_MOD}/mdk/system_nrf52833.c
    ${PROJ_FREERTOS_DIR}/QANI-FreeRTOS-Common/main.c
    ${PROJ_FREERTOS_DIR}/QANI-FreeRTOS-Common/hooks.c
    ${PROJ_FREERTOS_DIR}/QANI-FreeRTOS-Common/floor_current_hold.c
    ${PROJECT_DEFINITION_DIR}/ProjectName.c
)

list(APPEND SRC_FILES
    ${SDK_FRT}/source/croutine.c
    ${SDK_FRT}/source/event_groups.c
    ${SDK_FRT}/source/list.c
    ${SDK_FRT}/source/queue.c
    ${SDK_FRT}/source/stream_buffer.c
    ${SDK_FRT}/source/tasks.c
    ${SDK_FRT}/source/timers.c
    ${SDK_FRT}/source/portable/MemMang/heap_4.c
    ${SDK_FRT}/portable/GCC/nrf52/port.c
    ${SDK_FRT}/portable/CMSIS/nrf52/port_cmsis.c
    ${SDK_FRT}/portable/CMSIS/nrf52/port_cmsis_systick.c

    ${SDK_EXT}/fprintf/nrf_fprintf.c
    ${SDK_EXT}/fprintf/nrf_fprintf_format.c
    ${SDK_EXT}/segger_rtt/SEGGER_RTT.c
    ${SDK_EXT}/segger_rtt/SEGGER_RTT_printf.c
    ${SDK_EXT}/segger_rtt/SEGGER_RTT_Syscalls_GCC.c
)

foreach(f aes arc4 blowfish camellia ccm chacha20 chachapoly cipher cipher_wrap cmac des gcm platform_util poly1305)
    list(APPEND SRC_FILES ${SDK_MBED}/library/${f}.c)
endforeach()

list(APPEND SRC_FILES
    ${SDK_COMP}/boards/boards.c
    ${SDK_LIB}/fifo/app_fifo.c
    ${SDK_LIB}/uart/app_uart_fifo.c
    ${SDK_LIB}/util/app_util_platform.c
    ${SDK_LIB}/util/app_error.c
    ${SDK_LIB}/util/app_error_weak.c
    ${SDK_LIB}/util/app_error_handler_gcc.c
    ${SDK_LIB}/util/nrf_assert.c
    ${SDK_LIB}/atomic/nrf_atomic.c
    ${SDK_LIB}/queue/nrf_queue.c
    ${SDK_LIB}/button/app_button.c
    ${SDK_LIB}/timer/app_timer_freertos.c
    ${SDK_LIB}/fds/fds.c
    ${SDK_LIB}/hardfault/nrf52/handler/hardfault_handler_gcc.c
    ${SDK_LIB}/hardfault/hardfault_implementation.c
    ${SDK_LIB}/atomic_fifo/nrf_atfifo.c
    ${SDK_LIB}/atomic_flags/nrf_atflags.c
    ${SDK_LIB}/fstorage/nrf_fstorage.c
    ${SDK_LIB}/fstorage/nrf_fstorage_sd.c
    ${SDK_LIB}/experimental_section_vars/nrf_section_iter.c
    ${SDK_LIB}/strerror/nrf_strerror.c
    ${SDK_LIB}/sensorsim/sensorsim.c
    ${SDK_LIB}/ringbuf/nrf_ringbuf.c
    ${SDK_LIB}/memobj/nrf_memobj.c
    ${SDK_LIB}/balloc/nrf_balloc.c
    ${SDK_LIB}/pwr_mgmt/nrf_pwr_mgmt.c
    ${SDK_LIB}/bootloader/dfu/nrf_dfu_svci.c
    ${SDK_SVC}/ble_dfu/ble_dfu.c
    ${SDK_SVC}/ble_dfu/ble_dfu_unbonded.c

    ${SDK_LIB}/log/src/nrf_log_frontend.c
    ${SDK_LIB}/log/src/nrf_log_default_backends.c
    ${SDK_LIB}/log/src/nrf_log_backend_rtt.c
    ${SDK_LIB}/log/src/nrf_log_backend_serial.c
    ${SDK_LIB}/log/src/nrf_log_str_formatter.c

    ${SDK_LIB}/crypto/nrf_crypto_aes.c
    ${SDK_LIB}/crypto/nrf_crypto_rng.c
    ${SDK_LIB}/crypto/nrf_crypto_init.c
    ${SDK_LIB}/crypto/nrf_crypto_aead.c
    ${SDK_LIB}/crypto/backend/mbedtls/mbedtls_backend_aes.c
    ${SDK_LIB}/crypto/backend/mbedtls/mbedtls_backend_aes_aead.c
    ${SDK_LIB}/crypto/backend/nrf_hw/nrf_hw_backend_init.c
    ${SDK_LIB}/crypto/backend/nrf_hw/nrf_hw_backend_rng.c


    ${SDK_SD}/common/nrf_sdh.c
    ${SDK_SD}/common/nrf_sdh_ble.c
    ${SDK_SD}/common/nrf_sdh_freertos.c
    ${SDK_SD}/common/nrf_sdh_soc.c
)

foreach(f auth_status_tracker gatt_cache_manager gatts_cache_manager
          id_manager peer_data_storage peer_database peer_id
          peer_manager peer_manager_handler pm_buffer
          security_dispatcher security_manager)
    list(APPEND SRC_FILES ${SDK_BLE}/peer_manager/${f}.c)
endforeach()

foreach(f ble_advdata ble_conn_params ble_conn_state ble_srv_common)
    list(APPEND SRC_FILES ${SDK_BLE}/common/${f}.c)
endforeach()

list(APPEND SRC_FILES
    ${SDK_BLE}/ble_advertising/ble_advertising.c
    ${SDK_BLE}/nrf_ble_gatt/nrf_ble_gatt.c
    ${SDK_BLE}/nrf_ble_qwr/nrf_ble_qwr.c
    ${SDK_BLE}/nrf_ble_scan/nrf_ble_scan.c
    ${SDK_BLE}/ble_link_ctx_manager/ble_link_ctx_manager.c

    ${SDK_INT}/legacy/nrf_drv_spi.c
    ${SDK_INT}/legacy/nrf_drv_clock.c
    ${SDK_INT}/legacy/nrf_drv_power.c
    ${SDK_INT}/legacy/nrf_drv_uart.c
    ${SDK_INT}/legacy/nrf_drv_rng.c
)

foreach(f nrfx_saadc nrfx_rng nrfx_gpiote nrfx_rtc nrfx_spi nrfx_spim
          nrfx_timer nrfx_uart nrfx_uarte nrfx_power nrfx_wdt nrfx_clock
          nrfx_systick nrfx_pwm nrfx_twis)
    list(APPEND SRC_FILES ${SDK_MOD}/drivers/src/${f}.c)
endforeach()
list(APPEND SRC_FILES
    ${SDK_MOD}/drivers/src/prs/nrfx_prs.c
    ${SDK_MOD}/hal/nrf_nvmc.c
    ${SDK_MOD}/soc/nrfx_atomic.c

    ${SRC_ROOT}/EventManager/FreeRTOS/EventManager.c
    ${SRC_ROOT}/UWBUtils/util.c
    ${SRC_ROOT}/UWBUtils/translate.c
    ${QANI_APP}/reporter/rtt_reporter/rtt_reporter.c
    ${QANI_APP}/common/app/QANI/app.c
    ${QANI_APP}/common/config/debug_config.c
    ${QANI_APP}/common/defaultTask/FreeRTOS/create_default_task.c
    ${QANI_APP}/common/defaultTask/defaultTask.c
    ${QANI_APP}/common/thread/FreeRTOS/thread_fn.c
    ${QANI_APP}/fira/common_fira.c
    ${QANI_APP}/fira/fira_app_config.c
    ${QANI_APP}/fira/fira_dw3000.c
    ${QANI_APP}/fira/QANI/fira_niq.c
    ${SRC_ROOT}/OSAL/Src/FreeRTOS/alloc.c
    ${SRC_ROOT}/OSAL/Src/FreeRTOS/cmsis_os.c
    ${SRC_ROOT}/OSAL/Src/task_signal.c
    ${QANI_BRD}/Common_NRF/board.c
)

foreach(f HAL_error HAL_sleep HAL_DW3000 HAL_gpio HAL_timer
          HAL_RTC HAL_SPI HAL_uart HAL_watchdog HAL_power)
    list(APPEND SRC_FILES ${QANI_HAL}/${f}.c)
endforeach()

list(APPEND SRC_FILES
    ${SRC_ROOT}/UWB/dw3000_lp_mcu.c
    ${SRC_ROOT}/UWB/dw3000_mcps_mcu.c
    ${SRC_ROOT}/UWB/dw3000_calib_mcu.c
    ${SRC_ROOT}/UWB/dw3000_xtal_trim.c
    ${SRC_ROOT}/UWB/dw3000_statistics.c
    ${SRC_ROOT}/UWB/dw3000_pdoa.c
    ${SRC_ROOT}/UWB/uwbmac_platform.c
    ${SRC_ROOT}/UWB/FreeRTOS/create_mcps_Task_dw3000.c
    ${SRC_ROOT}/UWB/FreeRTOS/create_report_task.c

    ${QANI_BLE}/ble.c
    ${QANI_BLE}/nrf91_power_mode_manager.c
    ${QANI_BLE}/niq/ble_niq.c
    ${QANI_BLE}/qnis/ble_qnis.c
    ${QANI_BLE}/anis/ble_anis.c
    ${QANI_BUZ}/buzzer.c
    ${QANI_I2C}/i2c_nrf9151.c
    ${QANI_CFG}/persistent_config.c

    ${SRC_ROOT}/Helpers/crc16.c
    ${SRC_ROOT}/Helpers/cJSON.c
    ${SRC_ROOT}/Helpers/deca_dbg.c
    ${SRC_ROOT}/SecureComponent/Crypto/Nordic/mcps_crypto.c

    # Per-board
    ${SRC_ROOT}/AppConfig/${BOARD}/FreeRTOS/config.c
    ${QANI_BRD}/${BOARD}/rf_tuning_config.c
    ${QANI_BRD}/${BOARD}/FreeRTOS/QANI/${BOARD}.c
)

# ------------------------------------------------------------------
# Include paths
# ------------------------------------------------------------------
set(INC_FOLDERS
    ${PROJECT_DEFINITION_DIR}
    ${LIB_ROOT}/niq/Inc
    ${LIB_ROOT}/dwt_uwb_driver/Inc
    ${LIB_ROOT}/uwbstack/uwbstack_lib/include
    ${LIB_ROOT}/uwbstack/uwbstack_lib/include/compat/common
    ${LIB_ROOT}/uwbstack/uwbstack_lib/include/compat/cmsis
    ${LIB_ROOT}/uwbstack/uwbstack_lib/include/compat
    ${LIB_ROOT}/uwbstack/uwbstack_lib/include/mcps
    ${LIB_ROOT}/uwbstack/uwbstack_lib/include/uwbmac
    ${LIB_ROOT}/uwbstack/uwbstack_lib/include/uwb_driver_interface

    ${SRC_ROOT}/Apps/Inc
    ${QANI_APP}/common
    ${QANI_APP}/common/app/QANI
    ${QANI_APP}/common/config
    ${QANI_APP}/common/defaultTask
    ${QANI_APP}/common/thread
    ${QANI_APP}/fira
    ${QANI_APP}/fira/QANI
    ${QANI_APP}/reporter
    ${QANI_APP}/reporter/rtt_reporter
    ${SRC_ROOT}/Helpers/Inc
    ${QANI_BLE}
    ${QANI_BLE}/niq
    ${QANI_BLE}/qnis
    ${QANI_BLE}/anis
    ${QANI_BUZ}
    ${QANI_I2C}
    ${QANI_CFG}
    ${SRC_ROOT}/UWB
    ${SRC_ROOT}/UWB/FreeRTOS
    ${SRC_ROOT}/UWBUtils
    ${SRC_ROOT}/HAL/Inc
    ${SRC_ROOT}/Boards/Inc
    ${QANI_BRD}/${BOARD}/FreeRTOS
    ${QANI_BRD}/${BOARD}/FreeRTOS/Common
    ${QANI_BRD}/${BOARD}/FreeRTOS/QANI
    ${SRC_ROOT}/OSAL/Src/FreeRTOS
    ${SRC_ROOT}/OSAL/Inc
    ${SRC_ROOT}/AppConfig/Inc
    ${SRC_ROOT}/AppConfig/DefaultConfigs/QANI
    ${SRC_ROOT}/EventManager

    # SoftDevice headers first (so nrf_error.h resolves to the s140 one)
    ${SDK_SD}/common
    ${SDK_SD}/s140/headers
    ${SDK_SD}/s140/headers/nrf52
    ${SDK_SD}/mbr/headers

    ${SDK_ROOT}
    ${SDK_COMP}/boards
    ${SDK_COMP}/toolchain/cmsis/include
    ${SDK_EXT}/fprintf
    ${SDK_EXT}/segger_rtt
    ${SDK_EXT}/utf_converter
    ${SDK_MBED}/include
    ${SDK_FRT}/source/include
    ${SDK_FRT}/portable/GCC/nrf52
    ${SDK_FRT}/portable/CMSIS/nrf52
    ${SDK_INT}
    ${SDK_INT}/legacy
    ${SDK_MOD}
    ${SDK_MOD}/hal
    ${SDK_MOD}/drivers/include
    ${SDK_MOD}/mdk

    ${SDK_BLE}/common
    ${SDK_BLE}/ble_advertising
    ${SDK_BLE}/ble_link_ctx_manager
    ${SDK_BLE}/ble_dtm
    ${SDK_BLE}/ble_racp
    ${SDK_BLE}/nrf_ble_gatt
    ${SDK_BLE}/nrf_ble_qwr
    ${SDK_BLE}/nrf_ble_scan
    ${SDK_BLE}/peer_manager
)

foreach(svc ble_nus ble_ancs_c ble_ans_c ble_bas ble_bas_c ble_cscs
            ble_cts_c ble_dfu ble_dis ble_gls ble_hids ble_hrs
            ble_hrs_c ble_hts ble_ias ble_ias_c ble_lbs ble_lbs_c
            ble_lls ble_rscs ble_rscs_c ble_tps)
    list(APPEND INC_FOLDERS ${SDK_SVC}/${svc})
endforeach()

foreach(lib atomic atomic_fifo atomic_flags balloc bootloader bootloader/dfu bootloader/ble_dfu
            bsp button cli crc32 crypto csense csense_drv delay ecc
            experimental_section_vars experimental_task_manager
            fds fifo fstorage gfx gpiote hardfault hardfault/nrf52 hci
            led_softblink log log/src low_power_pwm mem_manager memobj
            mpu mutex pwm pwr_mgmt queue ringbuf scheduler sdcard
            sensorsim slip sortlist spi_mngr stack_guard stack_info
            strerror svc timer twi_mngr twi_sensor uart usbd
            usbd/class/cdc usbd/class/cdc/acm util)
    list(APPEND INC_FOLDERS ${SDK_LIB}/${lib})
endforeach()

foreach(be cc310 cifra mbedtls nrf_hw oberon optiga)
    list(APPEND INC_FOLDERS ${SDK_LIB}/crypto/backend/${be})
endforeach()

# ------------------------------------------------------------------
# Pre-built static libraries (linked --whole-archive in flags below)
# ------------------------------------------------------------------
set(QANI_PREBUILT_LIBS
    ${LIB_ROOT}/niq/libniq-m4-hfp-1.1.0.0.a
    ${LIB_ROOT}/dwt_uwb_driver/lib/libdwt_uwb_driver-m4-hfp-6.0.14.a
    ${LIB_ROOT}/uwbstack/uwbstack_lib/lib/cmsis_v1/embedded_mac/libembedded_mac_arm-m4-hfp_R11.9.2.a
)

# ------------------------------------------------------------------
# Defines / flags
# ------------------------------------------------------------------
set(COMMON_DEFINES
    DEBUG
    NRF_DEBUG
    BOARD_DWM3001_A1
    BOARD_CUSTOM
    NRF52833_XXAA
    NRF_SD_BLE_API_VERSION=7
    SOFTDEVICE_PRESENT
    S140
    NRF_CRYPTO_BACKEND_NRF_HW_RNG_ENABLED=1
    NRF_CRYPTO_ENABLED=1
    RNG_ENABLED=1
    INITIALIZE_USER_SECTIONS
    PROJECT_NAME=\"QANI\"
    ENABLE_USB_PRINT
    NRF_CRYPTO_BACKEND_MBEDTLS_ENABLED=1
    USE_RTT_PRINT=1
    CONFIG_GPIO_AS_PINRESET
    UWBMAC_VERSION=\"R11.9.2\"
    NRF_CRYPTO_BACKEND_NRF_HW_RNG_MBEDTLS_CTR_DRBG_ENABLED=0
    UWBSTACK
    USE_NIQ
    NRF_CRYPTO_ALLOCATOR=NRF_CRYPTO_ALLOCATOR_MALLOC
    MBEDTLS_CMAC_C
    UWBMAC_BUF_PLATFORM_H=\"uwbmac/uwbmac_buf_malloc.h\"
    FREERTOS
    NO_VTOR_CONFIG
    NRF_QUEUE_ENABLED=1
    NRF_CRYPTO_RNG_AUTO_INIT_ENABLED=1
    FLOAT_ABI_HARD
    BL_SETTINGS_ACCESS_ONLY
    NRF_DFU_SVCI_ENABLED
    NRF_DFU_TRANSPORT_BLE=1
)

set(CPU_FLAGS "-mcpu=cortex-m4 -mthumb -mabi=aapcs -mfloat-abi=hard -mfpu=fpv4-sp-d16")

# C and ASM share these
add_compile_options(
    -Os -g3
    -mcpu=cortex-m4 -mthumb -mabi=aapcs -mfloat-abi=hard -mfpu=fpv4-sp-d16
    -ffunction-sections -fdata-sections
    -Wall -Wno-implicit-function-declaration
)
add_compile_options($<$<COMPILE_LANGUAGE:C>:-std=c11>)
add_compile_definitions(${COMMON_DEFINES})

# ------------------------------------------------------------------
# Target
# ------------------------------------------------------------------
set(ELF_TARGET nrf52833_xxaa)

add_executable(${ELF_TARGET} ${SRC_FILES})
set_target_properties(${ELF_TARGET} PROPERTIES SUFFIX ".out" LINK_DEPENDS ${LINKER_SCRIPT})

target_include_directories(${ELF_TARGET} PRIVATE ${INC_FOLDERS})

target_link_options(${ELF_TARGET} PRIVATE
    -mcpu=cortex-m4 -mthumb -mabi=aapcs -mfloat-abi=hard -mfpu=fpv4-sp-d16
    -Os -g3
    -L${SDK_MOD}/mdk
    -T${LINKER_SCRIPT}
    -Wl,--gc-sections
    --specs=nano.specs
)

target_link_libraries(${ELF_TARGET} PRIVATE
    -Wl,--whole-archive
    ${QANI_PREBUILT_LIBS}
    -Wl,--no-whole-archive
    -lc -lnosys -lm
)

# Post-build: .hex and .bin
add_custom_command(TARGET ${ELF_TARGET} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O ihex   $<TARGET_FILE:${ELF_TARGET}> $<TARGET_FILE_DIR:${ELF_TARGET}>/${ELF_TARGET}.hex
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:${ELF_TARGET}> $<TARGET_FILE_DIR:${ELF_TARGET}>/${ELF_TARGET}.bin
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:${ELF_TARGET}>
    VERBATIM
)

# ------------------------------------------------------------------
# Flashing via JLinkExe
# ------------------------------------------------------------------
set(APP_HEX "$<TARGET_FILE_DIR:${ELF_TARGET}>/${ELF_TARGET}.hex")
set(JLINK       "JLinkExe" CACHE STRING "JLink commander binary")
set(JLINK_DEV   "nRF52833_xxAA" CACHE STRING "JLink target device")
set(JLINK_IF    "SWD" CACHE STRING "JLink interface")
set(JLINK_SPEED "4000" CACHE STRING "JLink speed (kHz)")
set(SERIAL      ""   CACHE STRING "J-Link serial number (optional)")

if(SERIAL)
    set(SERIAL_ARG "-SelectEmuBySN ${SERIAL}")
else()
    set(SERIAL_ARG "")
endif()

set(JLINK_BASE ${JLINK} -device ${JLINK_DEV} -if ${JLINK_IF} -speed ${JLINK_SPEED} -autoconnect 1 ${SERIAL_ARG})

# Bootloader settings page: tells the bootloader the J-Link-flashed app is valid.
set(BOOTLOADER_HEX "${PROJ_FREERTOS_DIR}/bootloader/_build/nrf52833_xxaa_s140.hex")
set(SETTINGS_HEX "${CMAKE_BINARY_DIR}/bl_settings.hex")
# Collar-wide packages/VERSION; 3.0.2 becomes 30002 for the bootloader.
set(VERSION_FILE "${REPO_ROOT}/../VERSION")
file(READ ${VERSION_FILE} VERSION_TEXT)
string(REGEX MATCH "VERSION_MAJOR = ([0-9]+)" _ ${VERSION_TEXT})
set(V_MAJOR ${CMAKE_MATCH_1})
string(REGEX MATCH "VERSION_MINOR = ([0-9]+)" _ ${VERSION_TEXT})
set(V_MINOR ${CMAKE_MATCH_1})
string(REGEX MATCH "PATCHLEVEL = ([0-9]+)" _ ${VERSION_TEXT})
math(EXPR APP_VERSION "${V_MAJOR} * 10000 + ${V_MINOR} * 100 + ${CMAKE_MATCH_1}")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${VERSION_FILE})
file(WRITE ${CMAKE_BINARY_DIR}/app_version ${APP_VERSION})

add_custom_target(bl_settings
    DEPENDS ${ELF_TARGET}
    COMMAND nrfutil nrf5sdk-tools settings generate --family NRF52 --application ${APP_HEX}
            --application-version ${APP_VERSION} --bootloader-version 1 --bl-settings-version 2
            --no-backup ${SETTINGS_HEX}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/flash_app.jlink
    CONTENT "r\nloadfile ${APP_HEX}\nloadfile ${SETTINGS_HEX}\nr\ng\nexit\n")
file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/flash_sd.jlink
    CONTENT "r\nerase\nw4 0x4001E504 1\nw4 0x10001208 0x5A\nw4 0x4001E504 0\nloadfile ${SOFTDEVICE_HEX}\nr\nexit\n")
# erase wipes UICR, which re-locks debug on newer nRF52833s; write APPROTECT = HwDisabled back.
file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/flash_full.jlink
    CONTENT "r\nerase\nw4 0x4001E504 1\nw4 0x10001208 0x5A\nw4 0x4001E504 0\nloadfile ${SOFTDEVICE_HEX}\nloadfile ${BOOTLOADER_HEX}\nloadfile ${APP_HEX}\nloadfile ${SETTINGS_HEX}\nr\ng\nexit\n")
file(GENERATE OUTPUT ${CMAKE_BINARY_DIR}/erase.jlink
    CONTENT "r\nerase\nexit\n")

add_custom_target(flash
    DEPENDS ${ELF_TARGET} bl_settings
    COMMAND ${JLINK_BASE} -CommanderScript flash_app.jlink
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(flash_softdevice
    COMMAND ${JLINK_BASE} -CommanderScript flash_sd.jlink
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(flash_full
    DEPENDS ${ELF_TARGET} bl_settings
    COMMAND ${JLINK_BASE} -CommanderScript flash_full.jlink
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(erase
    COMMAND ${JLINK_BASE} -CommanderScript erase.jlink
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

set(NRFUTIL "nrfutil" CACHE STRING "nrfutil binary")
if(SERIAL)
    set(NRFUTIL_SERIAL --serial-number ${SERIAL})
else()
    set(NRFUTIL_SERIAL "")
endif()

add_custom_target(recover
    COMMAND ${NRFUTIL} device recover --traits jlink ${NRFUTIL_SERIAL}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
