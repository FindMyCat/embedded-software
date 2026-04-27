NCS_VERSION := v3.0.0
BOARD       := nrf9160dk/nrf9160/ns

.PHONY: setup build flash clean

setup:
	west init -l manifest
	west update

build:
	nrfutil toolchain-manager launch --ncs-version $(NCS_VERSION) -- west build -b $(BOARD) app

RUNNER ?= jlink

flash:
	nrfutil toolchain-manager launch --ncs-version $(NCS_VERSION) -- west flash --runner $(RUNNER)

clean:
	rm -rf build
