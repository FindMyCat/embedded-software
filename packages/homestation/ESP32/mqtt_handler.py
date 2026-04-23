import ujson
from mqtt_as import MQTTClient, config as _base_config  # type: ignore — installed on device via mip

try:
    from typing import Optional
except ImportError:
    pass  # typing is not available on MicroPython at runtime


class MQTTHandler:
    def __init__(self, config: dict) -> None:
        self._topic: str = config["mqtt"]["topic"]
        self._location: dict = config["location"]

        # mqtt_as manages both WiFi and MQTT connection internally
        mqtt_config = dict(_base_config)
        mqtt_config.update({
            "client_id": config["mqtt"]["client_id"],
            "server": config["mqtt"]["broker"],
            "port": config["mqtt"]["port"],
            "ssid": config["wifi"]["ssid"],
            "wifi_pw": config["wifi"]["password"],
            "clean": True,
        })

        self._client = MQTTClient(mqtt_config)

    async def connect(self) -> None:
        """Connect to WiFi and MQTT broker. mqtt_as handles both."""
        await self._client.connect()
        print("[MQTT] Connected to broker")

    async def publish(self, battery: Optional[str]) -> None:
        payload_str: str = "{},{},{},{},{}".format(
            self._location["latitude"],
            self._location["longitude"],
            0,
            self._location["accuracy"],
            battery if battery else "0",
        )
        message: str = ujson.dumps({"msg": payload_str})
        await self._client.publish(self._topic, message, qos=1)
        print("[MQTT] Published:", message)
