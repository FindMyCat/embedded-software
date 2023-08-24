target_sources_ifdef(CONFIG_BOARD_NRF7002DK_NRF5340_CPUAPP
		     app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/network_wifi.c)
