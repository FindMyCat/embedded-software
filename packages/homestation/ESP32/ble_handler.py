import aioble  # type: ignore — installed on device via mip
import uasyncio as asyncio
import time
from micropython import const

try:
    from typing import Optional
except ImportError:
    pass  # typing is not available on MicroPython at runtime

# Advertising interval in microseconds (20ms)
_ADV_INTERVAL_US = const(20_000)

# BLE scan parameters
_SCAN_DURATION_MS = const(5_000)
_SCAN_INTERVAL_US = const(30_000)
_SCAN_WINDOW_US = const(30_000)

_STATUS_LOG_INTERVAL_MS = const(10_000)


class BLEHandler:
    def __init__(self, config: dict) -> None:
        self._collar_prefix: str = config["ble"]["collar_name_prefix"]
        self._homestation_name: str = config["ble"]["homestation_name"]
        self._presence_timeout_ms: int = config["ble"]["presence_timeout_ms"]
        self._last_seen_ms: Optional[int] = None
        self._last_battery: Optional[str] = None
        self._last_publish_ms: Optional[int] = None
        self._last_status_log_ms: int = 0

    async def advertise_loop(self) -> None:
        """Broadcast as FindMyCatHomeStation indefinitely so the collar can detect us."""
        print("[BLE] Advertising as:", self._homestation_name)
        while True:
            try:
                await aioble.advertise(
                    _ADV_INTERVAL_US,
                    name=self._homestation_name,
                    connectable=False,
                )
            except Exception as e:
                print("[BLE] Advertise error:", e)
                await asyncio.sleep(1)

    async def scan_loop(self) -> None:
        """Continuously scan for the collar and record its battery level."""
        while True:
            try:
                async with aioble.scan(
                    _SCAN_DURATION_MS,
                    interval_us=_SCAN_INTERVAL_US,
                    window_us=_SCAN_WINDOW_US,
                    active=True,
                ) as scanner:
                    async for result in scanner:
                        if result.name() and result.name().startswith(self._collar_prefix): ## TODO: Maybe we should do exact match rather than startswith?
                            self._last_seen_ms = time.ticks_ms()
                            mfr = result.manufacturer()
                            if mfr:
                                try:
                                    _, data = mfr
                                    self._last_battery = bytes(data).decode("utf-8").strip() ## TODO: Untested, need to test but in theory this should work (I think :D)
                                except Exception:
                                    pass
            except Exception as e:
                print("[BLE] Scan error:", e)

            now = time.ticks_ms()
            if time.ticks_diff(now, self._last_status_log_ms) >= _STATUS_LOG_INTERVAL_MS: ## Just so we don't spam the console with log events (useful when debugging)
                if self.collar_is_home():
                    print("[BLE] Collar Found")
                else:
                    print("[BLE] Collar not home")
                self._last_status_log_ms = now

    def collar_is_home(self) -> bool:
        """Return True if collar was seen within the presence timeout window."""
        if self._last_seen_ms is None:
            return False
        return time.ticks_diff(time.ticks_ms(), self._last_seen_ms) < self._presence_timeout_ms

    def should_publish(self) -> bool:
        """Return True if collar is home and we haven't published in the last 30 seconds."""
        if not self.collar_is_home():
            return False
        if self._last_publish_ms is None:
            return True
        return time.ticks_diff(time.ticks_ms(), self._last_publish_ms) >= self._presence_timeout_ms

    def mark_published(self) -> None:
        self._last_publish_ms = time.ticks_ms()

    def get_battery(self) -> Optional[str]:
        return self._last_battery
