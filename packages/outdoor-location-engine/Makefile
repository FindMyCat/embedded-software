NCS_VERSION  := v3.2.4
BOARD        := nrf9160dk/nrf9160/ns
BOARD_T91X   := thingy91x/nrf9151/ns
JLINK        := /Applications/SEGGER/JLink_V834

.PHONY: setup build build-dev build-prod build-dev-t91x build-prod-t91x flash flash-t91x flash-t91x-usb clean logs logs-t91x

setup:
	west init -l manifest
	west update

build: build-dev

build-dev:
	nrfutil toolchain-manager launch --ncs-version $(NCS_VERSION) -- west build -b $(BOARD) app -- -DEXTRA_CONF_FILE=overlay-dev.conf

build-prod:
	nrfutil toolchain-manager launch --ncs-version $(NCS_VERSION) -- west build -b $(BOARD) app -- -DEXTRA_CONF_FILE=overlay-prod.conf

build-dev-t91x:
	nrfutil toolchain-manager launch --ncs-version $(NCS_VERSION) -- west build -b $(BOARD_T91X) app -- "-DEXTRA_CONF_FILE=overlay-dev.conf;overlay-t91x.conf"

build-prod-t91x:
	nrfutil toolchain-manager launch --ncs-version $(NCS_VERSION) -- west build -b $(BOARD_T91X) app -- "-DEXTRA_CONF_FILE=overlay-prod.conf;overlay-t91x.conf"

RUNNER      ?= jlink
JLINK_SPEED ?= 500

flash:
	nrfutil toolchain-manager launch --ncs-version $(NCS_VERSION) -- west -v flash --runner $(RUNNER) --speed $(JLINK_SPEED)

recover: 
	west flash --recover
clean:
	rm -rf build

