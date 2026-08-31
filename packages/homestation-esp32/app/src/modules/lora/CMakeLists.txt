target_sources(app PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/rak3172.c
	${CMAKE_CURRENT_SOURCE_DIR}/lora_frame.c
	${CMAKE_CURRENT_SOURCE_DIR}/lora.c
)
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
