# Everything is done by the CMake project for the nRF52833, which is the part
# on the ANNA-B402. This only saves the cd; run `make help` for the targets.
#
# The DWM3001CDK tree alongside it is built from SEGGER Embedded Studio and has
# no CMake equivalent.

CMAKE_DIR := Projects/QANI/FreeRTOS/nrf52833dk/cmake

.DEFAULT_GOAL := build

# Anything not named here is forwarded as-is.
%:
	@$(MAKE) -C $(CMAKE_DIR) $@

build:
	@$(MAKE) -C $(CMAKE_DIR) build