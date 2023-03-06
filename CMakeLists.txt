#
# Copyright (c) 2021 Nordic Semiconductor
#
# SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
#

cmake_minimum_required(VERSION 3.20)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(location_sample)

zephyr_library_sources(src/main.c)

# Link all external dependencies
target_sources(app PRIVATE src/location.c)
target_sources(app PRIVATE src/location.h)

target_sources(app PRIVATE src/mqtt/mqtt.c)
target_sources(app PRIVATE src/mqtt/mqtt.h)