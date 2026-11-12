target_sources(app PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/provisioning.c
	${CMAKE_CURRENT_SOURCE_DIR}/provisioning_gatt.c
	${CMAKE_CURRENT_SOURCE_DIR}/provisioning_button.c
)
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
