-include .env

BUILD_DIR ?= build
GENERATOR ?= Ninja
FW_DIR    ?= /opt/cloud/firmware
DFU_ZIP   := $(BUILD_DIR)/indoor_update.zip

.PHONY: all configure build clean rebuild bootloader flash flash_softdevice flash_full erase recover fw-upload logs flash-logs help

all: build

$(BUILD_DIR)/build.ninja: CMakeLists.txt
	cmake -B $(BUILD_DIR) -G "$(GENERATOR)"

configure: $(BUILD_DIR)/build.ninja

build: configure
	cmake --build $(BUILD_DIR)

clean:
	cmake --build $(BUILD_DIR) --target clean 2>/dev/null || true

flash: build
	cmake --build $(BUILD_DIR) --target flash

flash_softdevice: configure
	cmake --build $(BUILD_DIR) --target flash_softdevice

bootloader:
	$(MAKE) -C ../../bootloader

flash_full: build bootloader recover
	cmake --build $(BUILD_DIR) --target flash_full

fw-upload: build
	@test -n "$(FW_SERVER)" || { echo "set FW_SERVER in .env (see .env.example)"; exit 1; }
	@test -f "$(DFU_KEY)" || { echo "set DFU_KEY in .env to your signing key"; exit 1; }
	nrfutil nrf5sdk-tools pkg generate --hw-version 52 --sd-req 0x0100 \
		--application $(BUILD_DIR)/nrf52833_xxaa.hex \
		--application-version $$(cat $(BUILD_DIR)/app_version) \
		--key-file $(DFU_KEY) $(DFU_ZIP)
	ssh $(FW_SERVER) "mkdir -p $(FW_DIR)"
	scp $(DFU_ZIP) $(FW_SERVER):$(FW_DIR)/indoor_update.zip.tmp
	ssh $(FW_SERVER) "mv $(FW_DIR)/indoor_update.zip.tmp $(FW_DIR)/indoor_update.zip && ls -l $(FW_DIR)/indoor_update.zip"

erase: configure
	cmake --build $(BUILD_DIR) --target erase

recover: configure
	cmake --build $(BUILD_DIR) --target recover

JLINK        ?= JLinkExe
JLINK_DEVICE ?= nRF52833_xxAA
RTT_SPEED    ?= 4000

logs:
	@set -m; \
	( tail -f /dev/null | $(JLINK) -device $(JLINK_DEVICE) -if SWD -speed $(RTT_SPEED) -autoconnect 1 >/dev/null 2>&1 ) & \
	SERVER_PGID=$$!; \
	trap "kill -- -$$SERVER_PGID 2>/dev/null; true" EXIT INT TERM; \
	sleep 2; \
	JLinkRTTClient || true

flash-logs: flash logs

help:
	@echo "Targets:"
	@echo "  build (default)   - configure (if needed) and build"
	@echo "  clean             - remove build artifacts"
	@echo "  rebuild           - wipe build/ and rebuild from scratch"
	@echo "  flash             - build + flash app via JLinkExe"
	@echo "  flash_softdevice  - flash S140 SoftDevice"
	@echo "  bootloader        - build the BLE DFU bootloader"
	@echo "  flash_full        - recover + build + flash SoftDevice + bootloader + app + reset"
	@echo "  fw-upload         - sign the app and upload it for BLE updates (needs .env)"
	@echo "  erase             - mass erase target"
	@echo "  recover           - nrfutil device recover (mass erase + clear AP-protect)"
	@echo "  logs              - stream RTT logs (JLinkExe + JLinkRTTClient)"
	@echo "  flash-logs        - flash then stream RTT logs"
	@echo ""
	@echo "Variables: BUILD_DIR ($(BUILD_DIR)), GENERATOR ($(GENERATOR)), JLINK_DEVICE ($(JLINK_DEVICE)), RTT_SPEED ($(RTT_SPEED))"
	@echo "Forward CMake vars with -D, e.g.: cmake -B $(BUILD_DIR) -DSERIAL=683123456"
