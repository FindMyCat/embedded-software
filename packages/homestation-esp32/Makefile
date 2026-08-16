BOARD  := esp32_devkitc_wroom/esp32/procpu
VENV   := .venv
PYTHON := python3
WEST   := $(VENV)/bin/west
PIP    := $(VENV)/bin/pip

.PHONY: setup build flash logs clean

# Creates venv and installs west into it
$(VENV):
	$(PYTHON) -m venv $(VENV)
	$(PIP) install west

setup: $(VENV)
	$(WEST) init -l manifest
	$(WEST) update
	$(PIP) install -r zephyr/scripts/requirements.txt
	$(WEST) blobs fetch hal_espressif
	$(WEST) sdk install
	$(WEST) zephyr-export

LOCAL_CONF := $(wildcard app/local.conf)
ifneq ($(LOCAL_CONF),)
BUILD_ARGS := -- -DEXTRA_CONF_FILE=local.conf
endif

build:
	$(WEST) build -b $(BOARD) app $(BUILD_ARGS)

flash:
	$(WEST) flash

logs:
	$(WEST) espressif monitor

clean:
	rm -rf build
