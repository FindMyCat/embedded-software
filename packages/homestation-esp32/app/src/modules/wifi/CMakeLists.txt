target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/wifi_scan_frame.c)
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(CONFIG_WIFI)
  target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/wifi_station.c)
endif()