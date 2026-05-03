NCS_VERSION := v3.2.4
BOARD       := nrf9160dk/nrf9160/ns
JLINK       := /Applications/SEGGER/JLink_V834

.PHONY: setup build build-dev build-prod flash clean logs

setup:
	west init -l manifest
	west update

build: build-dev

build-dev:
	nrfutil toolchain-manager launch --ncs-version $(NCS_VERSION) -- west build -b $(BOARD) app -- -DEXTRA_CONF_FILE=overlay-dev.conf

build-prod:
	nrfutil toolchain-manager launch --ncs-version $(NCS_VERSION) -- west build -b $(BOARD) app -- -DEXTRA_CONF_FILE=overlay-prod.conf

RUNNER ?= jlink

flash:
	nrfutil toolchain-manager launch --ncs-version $(NCS_VERSION) -- west flash --runner $(RUNNER)

clean:
	rm -rf build

logs:
	$(JLINK)/JLinkExe -device NRF9160_XXAA -if SWD -speed 4000 -autoconnect 1 &
	$(JLINK)/JLinkRTTClient
