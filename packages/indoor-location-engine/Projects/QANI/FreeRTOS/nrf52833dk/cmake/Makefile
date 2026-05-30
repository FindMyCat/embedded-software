# Convenience wrapper around CMake.
# Run any target from this directory: `make`, `make flash_full`, etc.

BUILD_DIR ?= build
GENERATOR ?= Ninja

.PHONY: all configure build clean rebuild flash flash_softdevice flash_full erase help

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

flash_full: build
	cmake --build $(BUILD_DIR) --target flash_full

erase: configure
	cmake --build $(BUILD_DIR) --target erase

help:
	@echo "Targets:"
	@echo "  build (default)   - configure (if needed) and build"
	@echo "  clean             - remove build artifacts"
	@echo "  rebuild           - wipe build/ and rebuild from scratch"
	@echo "  flash             - build + flash app via JLinkExe"
	@echo "  flash_softdevice  - flash S140 SoftDevice"
	@echo "  flash_full        - build + erase + flash SoftDevice + app + reset"
	@echo "  erase             - mass erase target"
	@echo ""
	@echo "Variables: BUILD_DIR ($(BUILD_DIR)), GENERATOR ($(GENERATOR))"
	@echo "Forward CMake vars with -D, e.g.: cmake -B $(BUILD_DIR) -DSERIAL=683123456"
