NCS_VERSION  := v3.2.4
BOARD        := nrf9160dk/nrf9160/ns
BOARD_T91X   := thingy91x/nrf9151/ns
BOARD_9151DK := nrf9151dk/nrf9151/ns
JLINK        := /Applications/SEGGER/JLink_V834

NCS   := nrfutil toolchain-manager launch --ncs-version $(NCS_VERSION) --
LOCAL := $(if $(wildcard app/local.conf),;local.conf)

.PHONY: setup build build-dev build-prod build-dev-t91x build-prod-t91x build-dev-9151dk build-prod-9151dk flash recover modem-fw-info modem-fw-flash logs flash-logs clean

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
# A factory or APPROTECT-locked part refuses SWD until it is mass erased. The
# jlink runner has no --recover, so unlock out of band and retry once.
flash:
	$(NCS) west -v flash --runner $(RUNNER) --speed $(JLINK_SPEED) \
		|| { $(MAKE) recover && $(NCS) west -v flash --runner $(RUNNER) --speed $(JLINK_SPEED); }

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
	nrfutil device recover --traits jlink

# Modem firmware version lives outside the application image, so a bad or
# missing one shows up as an early modem crash rather than a build failure.
modem-fw-info:
	nrfutil device device-info --traits jlink

MODEM_FW ?= $(lastword $(sort $(wildcard modem-fw/mfw_nrf91x1_*.zip)))

modem-fw-flash:
	@test -n "$(MODEM_FW)" || { echo "no modem-fw/mfw_nrf91x1_*.zip found"; exit 1; }
	nrfutil device program --firmware $(MODEM_FW) --traits jlink
clean:
	rm -rf build

