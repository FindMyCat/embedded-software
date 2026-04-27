NCS_VERSION := v2.5.0
BOARD       := nrf9160dk_nrf9160_ns

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
	nrfutil toolchain-manager launch --ncs-version $(NCS_VERSION) -- west build -t clean
