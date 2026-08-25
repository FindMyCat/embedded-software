import machine
import uasyncio as asyncio
import ujson
from ble_handler import BLEHandler
from mqtt_handler import MQTTHandler

def load_config() -> dict:
    with open("config.json") as f:
        return ujson.load(f)


async def publish_loop(ble: BLEHandler, mqtt: MQTTHandler, wdt: machine.WDT) -> None:
    """Check every 10 seconds if the collar is home and publish if needed."""
    while True:
        wdt.feed()
        await asyncio.sleep(10)

        if ble.should_publish():
            try:
                await mqtt.publish(ble.get_battery())
                ble.mark_published()
            except Exception as e:
                print("[MQTT] Publish error:", e)


async def periodic_trigger_loop(mqtt: MQTTHandler) -> None:
    """Publish a location update every 60 seconds regardless of collar presence (Testing Purposes)"""
    while True:
        await asyncio.sleep(60)
        print("[TRIGGER] Periodic trigger firing")
        try:
            await mqtt.publish(battery=None)
        except Exception as e:
            print("[TRIGGER] Publish error:", e)


async def main() -> None:
    config: dict = load_config()

    mqtt = MQTTHandler(config)
    ble = BLEHandler(config)

    await mqtt.connect()

    wdt = machine.WDT(timeout=30000)

    await asyncio.gather(
        ble.advertise_loop(),
        ble.scan_loop(),
        publish_loop(ble, mqtt, wdt),
        # periodic_trigger_loop(mqtt),
    )


asyncio.run(main())
