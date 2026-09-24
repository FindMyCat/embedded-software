BOARD  := adafruit_feather_esp32/esp32/procpu
VENV   := .venv
# Zephyr's build scripts call pathlib relative_to(walk_up=True), which is 3.12+.
PYTHON := python3.12
WEST   := $(VENV)/bin/west
PIP    := $(VENV)/bin/pip

export PATH := $(CURDIR)/$(VENV)/bin:$(PATH)

.PHONY: setup build flash logs clean

# Creates venv and installs west into it
$(VENV):
	$(PYTHON) -m venv $(VENV)
	$(PIP) install west

setup: $(VENV)
	$(WEST) init -l manifest
	$(WEST) update
	$(PIP) install -r zephyr/scripts/requirements-base.txt
	$(PIP) install "esptool>=5.0.2"
	$(WEST) blobs fetch hal_espressif
	$(WEST) sdk install
	$(WEST) zephyr-export

LOCAL_CONF := $(wildcard app/local.conf)
ifneq ($(LOCAL_CONF),)
BUILD_ARGS := -- -DEXTRA_CONF_FILE=local.conf
endif

build:
	$(WEST) build -b $(BOARD) app $(BUILD_ARGS)

# 921600 loses sync on this USB-serial chip
flash:
	$(WEST) flash --esp-baud-rate 115200

logs:
	$(WEST) espressif monitor

clean:
	rm -rf build
