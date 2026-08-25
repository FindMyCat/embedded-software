# Homestation ESP32 Setup

## Requirements

- ESP32 with MicroPython v1.24.1+ flashed
- VS Code with MicroPico extension

## 1. Install packages on the ESP32

Connect to the board via the MicroPico REPL and run the following. The board must be connected to WiFi first.

```python
# Connect to WiFi
import network
wlan = network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect("your-ssid", "your-password")

# Install packages
import mip
mip.install("aioble")
```

## 2. Configure

Edit `config.json` with your settings before uploading:

```json
{
  "wifi": {
    "ssid": "your-wifi-ssid",
    "password": "your-wifi-password"
  },
  "mqtt": {
    "broker": "your-mqtt-broker-hostname",
    "port": 1883,
    "topic": "/your-traccar-device-topic",
    "client_id": "your-findmycat-device-imei"
  },
  "location": {
    "latitude": 0.0,
    "longitude": 0.0,
    "accuracy": 5.00
  },
  "ble": {
    "collar_name_prefix": "FMC", ## This MUST correspond to the collars advertising BLE packet on the indoor location engine DEFAULT: FMC
    "homestation_name": "FindMyCatHomeStation", ## This MUST correspond to what the indoor location engine is looking for DEFAULT: FindMyCatHomeStation
    "presence_timeout_ms": 30000 ## Don't change this if you're not sure what you're doing
  }
}
```

## 3. Upload files

Right-click the project root in VS Code and select **Upload project to Pico**. This uploads all files preserving the directory structure, including `lib/mqtt_as/__init__.py`.

## 4. Verify files on the board

In the MicroPico REPL, confirm all files landed correctly:

```python
import os

# Root — should contain main.py, ble_handler.py, mqtt_handler.py, config.json
os.listdir("/")

# lib — should contain mqtt_as
os.listdir("/lib")

# mqtt_as package — should contain __init__.py
os.listdir("/lib/mqtt_as")
```

## 5. Run

Reset the board (press EN/RST or run `machine.reset()` in the REPL). You should see:

```
[MQTT] Connected to broker
[BLE] Advertising as: FindMyCatHomeStation
[BLE] Collar not home
```

When the collar comes into range:

```
[BLE] Collar Found
[BLE] Collar home
[MQTT] Published: {"msg": "..."}
```

## Testing without a collar

To verify the WiFi → MQTT pipeline without a collar nearby, uncomment `periodic_trigger_loop` in `main.py`. This publishes a location update every 60 seconds regardless of BLE state.

---

## Additional Info

- **mqtt_as** (Peter Hinch) — the MQTT client library bundled in `lib/mqtt_as/`. Handles WiFi and MQTT reconnection automatically: https://github.com/peterhinch/micropython-mqtt
- **aioble** (MicroPython) — official async BLE library, installed via `mip.install("aioble")`: https://github.com/micropython/micropython-lib/tree/master/micropython/bluetooth/aioble
