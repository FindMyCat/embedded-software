#
# Copyright (c) 2021 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(find_my_cat)

zephyr_library_sources(src/main.c)

# Link all external dependencies

target_sources(app PRIVATE src/LocationEngine/LocationEngine.c)
target_sources(app PRIVATE src/LocationEngine/LocationEngine.h)

target_sources(app PRIVATE src/SmsListener/SmsListener.c)
target_sources(app PRIVATE src/SmsListener/SmsListener.h)

target_sources(app PRIVATE src/Dispatcher/Dispatcher.c)
target_sources(app PRIVATE src/Dispatcher/Dispatcher.h)


target_sources(app PRIVATE src/Responder/Responder.c)
target_sources(app PRIVATE src/Responder/Responder.h)

target_sources(app PRIVATE src/mqtt/mqttsn.c)
target_sources(app PRIVATE src/mqtt/mqttsn.h)
