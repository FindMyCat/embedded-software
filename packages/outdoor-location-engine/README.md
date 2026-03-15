# FindMyCat - Outdoor Location Engine

Hardware: nrf9160-DK

Software Architecture:

![arch](./arch.png)

## Getting Started

### 1. Install nRF Connect SDK

1. Download and install [nRF Connect for Desktop](https://www.nordicsemi.com/Products/Development-tools/nRF-Connect-for-Desktop)
2. Open the **Toolchain Manager** inside it
3. Install **nRF Connect SDK v2.3.0**

### 2. Set up your terminal

Run this every time you open a new terminal:

```bash
source scripts/env.sh
```

### 3. Build

```bash
west build -b nrf9160dk_nrf9160_ns
```

### 4. Flash

Plug in your nRF9160-DK via USB, then:

```bash
west flash
```

### Troubleshooting

Build fails with a cmake cache error - do a clean build:
```bash
west build -b nrf9160dk_nrf9160_ns --pristine
```

`west: command not found` - you forgot to source the env:
```bash
source scripts/env.sh
```

### Editor setup (Neovim + clangd)

Run this once after cloning to generate `compile_commands.json` for LSP support:

```bash
source scripts/env.sh
west build -b nrf9160dk_nrf9160_ns -- -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
```

The `.clangd` config is already in the repo, clangd picks it up from there.

## MQTT Data format
When sending data over mqtt the following format is used to send a comma separated string to save as much data as possible.

latitude,longitude,satellites_tracked,accuracy
