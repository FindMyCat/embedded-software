target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/wifi_scan_frame.c)
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(CONFIG_WIFI)
  target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/wifi_station.c)
endif()

if(CONFIG_WIFI_GEOLOCATION)
  target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/wifi_geolocation.c)

  generate_inc_file_for_target(app
    ${APPLICATION_SOURCE_DIR}/certs/gts_root_r1.pem
    ${ZEPHYR_BINARY_DIR}/include/generated/gts_root_r1.pem.inc)
endif()
