/* Linker script for nRF52833 + SoftDevice S140 7.2.0 (QANI/FreeRTOS).
 * Ported from ses/flash_placement.xml. Memory layout matches the SES
 * linker_section_placement_macros from DWM3001CDK-QANI-FreeRTOS.emProject.
 *
 * FLASH layout:
 *   0x00000 .. 0x26FFF : SoftDevice S140 (reserved)
 *   0x27000 .. 0x27FFF : .vectors (app vector table)
 *   0x28000 .. 0x283FF : default config blob (DEFAULT_CONFIG)  [data island]
 *   0x29000 .. 0x29FFF : .fconfig (writable config page, fixed addr)
 *   0x2A000 .. end     : .init / .text / .rodata / app
 *
 * RAM layout:
 *   0x20000000 .. 0x20002B37 : SoftDevice RAM (reserved)
 *   0x20002B38 .. 0x2001FFFF : app RAM (~117 KiB)
 */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH_VECTORS (rx) : ORIGIN = 0x27000,    LENGTH = 0x2000
  FCONFIG       (r)  : ORIGIN = 0x29000,    LENGTH = 0x1000
  FLASH         (rx) : ORIGIN = 0x2A000,    LENGTH = 0x56000
  RAM          (rwx) : ORIGIN = 0x20002B38, LENGTH = 0x1D4C8
}

SECTIONS
{
  /* App vector table at 0x27000. Pulled out of nrf_common.ld's .text so
   * the rest of code can live above the fconfig page at 0x29000. */
  .vectors 0x27000 :
  {
    KEEP(*(.isr_vector))
  } > FLASH_VECTORS

  /* Fixed-address writable config page (FDS-like). KEEP so GC doesn't
   * drop the symbols even when nothing references them at link time. */
  .fconfig 0x29000 :
  {
    __fconfig_start = .;
    KEEP(*(.fconfig))
    KEEP(*(.fconfig.*))
    . = ALIGN(4);
    __fconfig_end = .;
  } > FCONFIG
} INSERT BEFORE .text;

/* Referenced by nrf_sdh_ble.c to tell the SoftDevice where app RAM begins.
 * Must equal ORIGIN(RAM) (= RAM_START in the SES macros). */
PROVIDE(__app_ram_start__ = ORIGIN(RAM));

SECTIONS
{
  . = ALIGN(4);
  .mem_section_dummy_ram :
  {
  }

  /* RAM-resident SoftDevice / log scratch tables */
  .cli_sorted_cmd_ptrs :
  {
    PROVIDE(__start_cli_sorted_cmd_ptrs = .);
    KEEP(*(.cli_sorted_cmd_ptrs))
    PROVIDE(__stop_cli_sorted_cmd_ptrs = .);
  } > RAM
  .fs_data :
  {
    PROVIDE(__start_fs_data = .);
    KEEP(*(.fs_data))
    PROVIDE(__stop_fs_data = .);
  } > RAM
  .log_dynamic_data :
  {
    PROVIDE(__start_log_dynamic_data = .);
    KEEP(*(SORT(.log_dynamic_data*)))
    PROVIDE(__stop_log_dynamic_data = .);
  } > RAM
  .log_filter_data :
  {
    PROVIDE(__start_log_filter_data = .);
    KEEP(*(SORT(.log_filter_data*)))
    PROVIDE(__stop_log_filter_data = .);
  } > RAM

  /* Qorvo runtime config in RAM. Not loaded; just symbols. */
  .rconfig (NOLOAD) :
  {
    . = ALIGN(4);
    __rconfig_start = .;
    KEEP(*(.rconfig))
    . = ALIGN(4);
    __rconfig_end = .;
    KEEP(*(.rconfig_crc))
    . = ALIGN(4);
    __rconfig_crc_end = .;
  } > RAM
} INSERT AFTER .data;

SECTIONS
{
  .mem_section_dummy_rom :
  {
  }

  /* Nordic SDK SoftDevice handler observers + housekeeping tables */
  .sdh_soc_observers :
  {
    PROVIDE(__start_sdh_soc_observers = .);
    KEEP(*(SORT(.sdh_soc_observers*)))
    PROVIDE(__stop_sdh_soc_observers = .);
  } > FLASH
  .sdh_ble_observers :
  {
    PROVIDE(__start_sdh_ble_observers = .);
    KEEP(*(SORT(.sdh_ble_observers*)))
    PROVIDE(__stop_sdh_ble_observers = .);
  } > FLASH
  .sdh_state_observers :
  {
    PROVIDE(__start_sdh_state_observers = .);
    KEEP(*(SORT(.sdh_state_observers*)))
    PROVIDE(__stop_sdh_state_observers = .);
  } > FLASH
  .sdh_stack_observers :
  {
    PROVIDE(__start_sdh_stack_observers = .);
    KEEP(*(SORT(.sdh_stack_observers*)))
    PROVIDE(__stop_sdh_stack_observers = .);
  } > FLASH
  .sdh_req_observers :
  {
    PROVIDE(__start_sdh_req_observers = .);
    KEEP(*(SORT(.sdh_req_observers*)))
    PROVIDE(__stop_sdh_req_observers = .);
  } > FLASH
  .pwr_mgmt_data :
  {
    PROVIDE(__start_pwr_mgmt_data = .);
    KEEP(*(SORT(.pwr_mgmt_data*)))
    PROVIDE(__stop_pwr_mgmt_data = .);
  } > FLASH
  .nrf_queue :
  {
    PROVIDE(__start_nrf_queue = .);
    KEEP(*(.nrf_queue))
    PROVIDE(__stop_nrf_queue = .);
  } > FLASH
  .nrf_balloc :
  {
    PROVIDE(__start_nrf_balloc = .);
    KEEP(*(.nrf_balloc))
    PROVIDE(__stop_nrf_balloc = .);
  } > FLASH
  .cli_command :
  {
    PROVIDE(__start_cli_command = .);
    KEEP(*(.cli_command))
    PROVIDE(__stop_cli_command = .);
  } > FLASH
  .crypto_data :
  {
    PROVIDE(__start_crypto_data = .);
    KEEP(*(SORT(.crypto_data*)))
    PROVIDE(__stop_crypto_data = .);
  } > FLASH
  .log_const_data :
  {
    PROVIDE(__start_log_const_data = .);
    KEEP(*(SORT(.log_const_data*)))
    PROVIDE(__stop_log_const_data = .);
  } > FLASH
  .log_backends :
  {
    PROVIDE(__start_log_backends = .);
    KEEP(*(SORT(.log_backends*)))
    PROVIDE(__stop_log_backends = .);
  } > FLASH

  /* Qorvo / QANI specific tables (registered drivers, apps, configs). */
  .dw_drivers :
  {
    PROVIDE(__dw_drivers_start = .);
    KEEP(*dw3000_device.c.obj(.dw_drivers))
    KEEP(*dw3000_device.c.obj(SORT(.dw_drivers.*)))
    PROVIDE(__dw_drivers_end = .);
  } > FLASH
  .known_apps :
  {
    PROVIDE(__known_apps_start = .);
    KEEP(*(.known_apps))
    KEEP(*(SORT(.known_apps.*)))
    PROVIDE(__known_apps_end = .);
  } > FLASH
  .config_entry :
  {
    PROVIDE(__config_entry_start = .);
    KEEP(*(.config_entry))
    KEEP(*(SORT(.config_entry.*)))
    PROVIDE(__config_entry_end = .);
  } > FLASH
} INSERT AFTER .text

INCLUDE "nrf_common.ld"
