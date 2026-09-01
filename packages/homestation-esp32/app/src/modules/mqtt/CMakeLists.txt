target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(CONFIG_MQTT_PUBLISHER)
  target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/mqtt_publisher.c)
endif()