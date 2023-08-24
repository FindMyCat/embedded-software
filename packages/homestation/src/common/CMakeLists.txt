# Include message_channel.h
target_include_directories(app PRIVATE .)

target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/message_channel.c)
