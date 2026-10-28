NCS_VERSION  := v3.2.4
BOARD        := nrf9160dk/nrf9160/ns
BOARD_T91X   := thingy91x/nrf9151/ns
BOARD_9151DK := nrf9151dk/nrf9151/ns
JLINK        := /Applications/SEGGER/JLink_V834

NCS   := nrfutil toolchain-manager launch --ncs-version $(NCS_VERSION) --
LOCAL := $(if $(wildcard app/local.conf),;local.conf)

.PHONY: setup build-dev-t91x build-prod-t91x build-dev-9151dk build-prod-9151dk flash recover modem-fw-info modem-fw-flash logs logs-mcuboot clean

setup:
	west init -l manifest
	west update

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
	$(NCS) west -v flash --runner $(RUNNER) --speed $(JLINK_SPEED) \
		|| { $(MAKE) recover && $(NCS) west -v flash --runner $(RUNNER) --speed $(JLINK_SPEED); }

JLINK_DEVICE ?= nRF9151_xxCA
RTT_SPEED    ?= 4000

# MCUboot and the app each leave an RTT block in RAM, and an open search
# latches onto the first. They live in fixed regions: MCUboot in the 32 KB
# TF-M later takes, the app in the non-secure SRAM after it.
RTT_RANGE_MCUBOOT := 0x20000000 0x8000
RTT_RANGE_APP     := 0x20008000 0x38000

define rtt_client
	@set -m; \
	( { echo "exec SetRTTSearchRanges $(1)"; tail -f /dev/null; } | $(JLINK)/JLinkExe -device $(JLINK_DEVICE) -if SWD -speed $(RTT_SPEED) -autoconnect 1 >/dev/null 2>&1 ) & \
	SERVER_PGID=$$!; \
	trap "kill -- -$$SERVER_PGID 2>/dev/null; true" EXIT INT TERM; \
	sleep 2; \
	$(JLINK)/JLinkRTTClient || true
endef

logs:
	$(call rtt_client,$(RTT_RANGE_APP))

logs-mcuboot:
	$(call rtt_client,$(RTT_RANGE_MCUBOOT))

recover:
	nrfutil device recover --traits jlink

modem-fw-info:
	nrfutil device device-info --traits jlink

MODEM_FW ?= $(lastword $(sort $(wildcard modem-fw/mfw_nrf91x1_*.zip)))

modem-fw-flash:
	@test -n "$(MODEM_FW)" || { echo "no modem-fw/mfw_nrf91x1_*.zip found"; exit 1; }
	nrfutil device program --firmware $(MODEM_FW) --traits jlink
clean:
	rm -rf build

