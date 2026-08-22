NCS_VERSION := v3.2.4
BOARD       := nrf9160dk/nrf9160/ns
JLINK       := /Applications/SEGGER/JLink_V834

NCS := nrfutil toolchain-manager launch --ncs-version $(NCS_VERSION) --

.PHONY: setup build flash logs recover clean

setup:
	west init -l manifest
	west update

build:
	$(NCS) west build -b $(BOARD) app

RUNNER      ?= jlink
JLINK_SPEED ?= 1000

flash:
	$(NCS) west -v flash --runner $(RUNNER) --speed $(JLINK_SPEED)

# JLinkExe holds the debug connection and serves RTT on localhost:19021, so it
# is backgrounded here and torn down on exit, leaving the client in the
# foreground. Its stdin must stay open or it exits, hence the tail.
# Override for a 9151 board: make logs JLINK_DEVICE=nRF9151_xxCA
JLINK_DEVICE ?= nRF9160_xxAA
RTT_SPEED    ?= 4000

logs:
	@set -m; \
	( tail -f /dev/null | $(JLINK)/JLinkExe -device $(JLINK_DEVICE) -if SWD -speed $(RTT_SPEED) -autoconnect 1 >/dev/null 2>&1 ) & \
	SERVER_PGID=$$!; \
	trap "kill -- -$$SERVER_PGID 2>/dev/null; true" EXIT INT TERM; \
	sleep 2; \
	$(JLINK)/JLinkRTTClient || true

recover:
	$(NCS) west flash --recover

clean:
	rm -rf build
