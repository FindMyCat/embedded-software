NCS_VERSION  := v3.2.4
BOARD        := nrf9160dk/nrf9160/ns
BOARD_T91X   := thingy91x/nrf9151/ns
BOARD_9151DK := nrf9151dk/nrf9151/ns
JLINK        := /Applications/SEGGER/JLink_V834

NCS   := nrfutil toolchain-manager launch --ncs-version $(NCS_VERSION) --
LOCAL := $(if $(wildcard app/local.conf),;local.conf)

.PHONY: setup build build-dev build-prod build-dev-t91x build-prod-t91x build-dev-9151dk build-prod-9151dk flash logs flash-logs clean

setup:
	west init -l manifest
	west update

build: build-dev

build-dev:
	$(NCS) west build -b $(BOARD) app -- "-DEXTRA_CONF_FILE=overlay-dev.conf$(LOCAL)"

build-prod:
	$(NCS) west build -b $(BOARD) app -- "-DEXTRA_CONF_FILE=overlay-prod.conf$(LOCAL)"

build-dev-t91x:
	$(NCS) west build -b $(BOARD_T91X) app -- "-DEXTRA_CONF_FILE=overlay-dev.conf;overlay-t91x.conf$(LOCAL)"

build-prod-t91x:
	$(NCS) west build -b $(BOARD_T91X) app -- "-DEXTRA_CONF_FILE=overlay-prod.conf;overlay-t91x.conf$(LOCAL)"

build-dev-9151dk:
	$(NCS) west build -b $(BOARD_9151DK) app -- "-DEXTRA_CONF_FILE=overlay-dev.conf;overlay-9151dk.conf$(LOCAL)"

build-prod-9151dk:
	$(NCS) west build -b $(BOARD_9151DK) app -- "-DEXTRA_CONF_FILE=overlay-prod.conf;overlay-9151dk.conf$(LOCAL)"

RUNNER      ?= jlink
JLINK_SPEED ?= 1000

flash:
	nrfutil toolchain-manager launch --ncs-version $(NCS_VERSION) -- west -v flash --runner $(RUNNER) --speed $(JLINK_SPEED)

# JLinkExe holds the debug connection and serves RTT on localhost:19021, so it
# is backgrounded here and torn down on exit, leaving the client in the
# foreground. Override for the 9160: make logs JLINK_DEVICE=nRF9160_xxAA
JLINK_DEVICE ?= nRF9151_xxCA
RTT_SPEED    ?= 4000

logs:
	@set -m; \
	( tail -f /dev/null | $(JLINK)/JLinkExe -device $(JLINK_DEVICE) -if SWD -speed $(RTT_SPEED) -autoconnect 1 >/dev/null 2>&1 ) & \
	SERVER_PGID=$$!; \
	trap "kill -- -$$SERVER_PGID 2>/dev/null; true" EXIT INT TERM; \
	sleep 2; \
	$(JLINK)/JLinkRTTClient || true

recover:
	west flash --recover
clean:
	rm -rf build

