target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(CONFIG_BLE_SCANNER)
  target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ble_scanner.c)
endif()

if(CONFIG_BLE_BEACON)
  target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/ble_beacon.c)
endif()