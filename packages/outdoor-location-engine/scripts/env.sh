#!/bin/bash
# Source this file to set up the nRF Connect SDK environment in your current shell:
#   source scripts/env.sh
#
# Requirements: nRF Connect SDK v2.3.0 installed via nRF Connect for Desktop
#   macOS: /opt/nordic/ncs/v2.3.0
#   Linux: ~/ncs/v2.3.0

NCS_VERSION="v2.3.0"

if [ -d "/opt/nordic/ncs/${NCS_VERSION}" ]; then
  NCS_BASE="/opt/nordic/ncs"
elif [ -d "${HOME}/ncs/${NCS_VERSION}" ]; then
  NCS_BASE="${HOME}/ncs"
else
  echo "ERROR: nRF Connect SDK ${NCS_VERSION} not found."
  echo "Install it via nRF Connect for Desktop, then update NCS_BASE in this script."
  return 1
fi

export ZEPHYR_BASE="${NCS_BASE}/${NCS_VERSION}/zephyr"
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
export ZEPHYR_SDK_INSTALL_DIR="${NCS_BASE}/toolchains/${NCS_VERSION}/opt/zephyr-sdk"
export PATH="${NCS_BASE}/toolchains/${NCS_VERSION}/bin:${PATH}"

echo "nRF Connect SDK ${NCS_VERSION} environment ready."
echo "  ZEPHYR_BASE=${ZEPHYR_BASE}"
