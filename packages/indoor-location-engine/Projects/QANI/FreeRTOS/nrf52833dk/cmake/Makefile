# Convenience wrapper around CMake.
# Run any target from this directory: `make`, `make flash_full`, etc.

BUILD_DIR ?= build
GENERATOR ?= Ninja

.PHONY: all configure build clean rebuild flash flash_softdevice flash_full erase recover logs flash-logs help

all: build

$(BUILD_DIR)/build.ninja: CMakeLists.txt
	cmake -B $(BUILD_DIR) -G "$(GENERATOR)"

configure: $(BUILD_DIR)/build.ninja

build: configure
	cmake --build $(BUILD_DIR)

clean:
	cmake --build $(BUILD_DIR) --target clean 2>/dev/null || true

rebuild:
	rm -rf $(BUILD_DIR)
	$(MAKE) build

flash: build
	cmake --build $(BUILD_DIR) --target flash

flash_softdevice: configure
	cmake --build $(BUILD_DIR) --target flash_softdevice

flash_full: build recover
	cmake --build $(BUILD_DIR) --target flash_full

erase: configure
	cmake --build $(BUILD_DIR) --target erase

recover: configure
	cmake --build $(BUILD_DIR) --target recover

# JLinkExe holds the debug connection and serves RTT on localhost:19021, so it
# is backgrounded here and torn down on exit, leaving the client in the
# foreground.
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
	@echo "  flash_full        - recover + build + flash SoftDevice + app + reset"
	@echo "  erase             - mass erase target"
	@echo "  recover           - nrfutil device recover (mass erase + clear AP-protect)"
	@echo "  logs              - stream RTT logs (JLinkExe + JLinkRTTClient)"
	@echo "  flash-logs        - flash then stream RTT logs"
	@echo ""
	@echo "Variables: BUILD_DIR ($(BUILD_DIR)), GENERATOR ($(GENERATOR)), JLINK_DEVICE ($(JLINK_DEVICE)), RTT_SPEED ($(RTT_SPEED))"
	@echo "Forward CMake vars with -D, e.g.: cmake -B $(BUILD_DIR) -DSERIAL=683123456"
