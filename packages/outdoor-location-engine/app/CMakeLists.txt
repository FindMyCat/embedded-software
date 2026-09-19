cmake_minimum_required(VERSION 3.20)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(find_my_cat)

zephyr_library_sources(src/main.c)

# Link all external dependencies

target_sources(app PRIVATE src/LocationEngine/LocationEngine.c)
target_sources(app PRIVATE src/LocationEngine/LocationEngine.h)

target_sources(app PRIVATE src/SmsListener/SmsListener.c)
target_sources(app PRIVATE src/SmsListener/SmsListener.h)

target_sources(app PRIVATE src/UDPListener/UDPListener.c)
target_sources(app PRIVATE src/UDPListener/UDPListener.h)

target_sources(app PRIVATE src/UDPListener/NewUDPListener.c)
target_sources(app PRIVATE src/UDPListener/NewUDPListener.h)

target_sources(app PRIVATE src/Dispatcher/Dispatcher.c)
target_sources(app PRIVATE src/Dispatcher/Dispatcher.h)


target_sources(app PRIVATE src/Responder/Responder.c)
target_sources(app PRIVATE src/Responder/Responder.h)

target_sources(app PRIVATE src/mqtt/mqttsn.c)
target_sources(app PRIVATE src/mqtt/mqttsn.h)

target_sources(app PRIVATE src/utils/network.c)
target_sources(app PRIVATE src/utils/network.h)

target_sources(app PRIVATE src/LTE/lte.c)
target_sources(app PRIVATE src/LTE/lte.h)

target_sources(app PRIVATE src/LoRa/rak3172.c)
target_sources(app PRIVATE src/LoRa/rak3172.h)

target_sources(app PRIVATE src/LoRa/lora_frame.c)
target_sources(app PRIVATE src/LoRa/lora_frame.h)

target_sources_ifdef(CONFIG_LORA_LINK app PRIVATE src/LoRa/lora_link.c)
target_sources_ifdef(CONFIG_LORA_LINK app PRIVATE src/LoRa/lora_link.h)

target_sources(app PRIVATE src/comm/transport.c)
target_sources(app PRIVATE src/comm/transport.h)

target_sources(app PRIVATE src/WiFi/wifi_location.c)
target_sources(app PRIVATE src/WiFi/wifi_location.h)

target_sources(app PRIVATE src/WiFi/wifi_scan.c)
target_sources(app PRIVATE src/WiFi/wifi_scan.h)

target_sources(app PRIVATE src/FuelGauge/fuel_gauge.c)
target_sources(app PRIVATE src/FuelGauge/fuel_gauge.h)

target_sources(app PRIVATE src/led/led.c)
target_sources(app PRIVATE src/led/led.h)

target_sources(app PRIVATE src/shared.h)

# Diagnostics related sources.
target_sources_ifdef(CONFIG_FINDMYCAT_DIAGNOSTICS app PRIVATE src/Diagnostics/diagnostics.c)
target_sources_ifdef(CONFIG_FINDMYCAT_DIAGNOSTICS app PRIVATE src/Diagnostics/diagnostics.h)
target_sources_ifdef(CONFIG_FINDMYCAT_DIAGNOSTICS app PRIVATE src/FuelGauge/diagnostics.c)
target_sources_ifdef(CONFIG_FINDMYCAT_DIAGNOSTICS app PRIVATE src/FuelGauge/diagnostics.h)
target_sources_ifdef(CONFIG_FINDMYCAT_DIAGNOSTICS app PRIVATE src/LTE/diagnostics.c)
target_sources_ifdef(CONFIG_FINDMYCAT_DIAGNOSTICS app PRIVATE src/LTE/diagnostics.h)
target_sources_ifdef(CONFIG_FINDMYCAT_DIAGNOSTICS app PRIVATE src/comm/diagnostics.c)
target_sources_ifdef(CONFIG_FINDMYCAT_DIAGNOSTICS app PRIVATE src/comm/nrf52_diagnostics.c)
target_sources_ifdef(CONFIG_FINDMYCAT_DIAGNOSTICS app PRIVATE src/comm/nrf52_diagnostics.h)
target_sources_ifdef(CONFIG_FINDMYCAT_DIAGNOSTICS app PRIVATE src/WiFi/diagnostics.c)
target_sources_ifdef(CONFIG_FINDMYCAT_DIAGNOSTICS app PRIVATE src/WiFi/diagnostics.h)
target_sources_ifdef(CONFIG_FINDMYCAT_DIAGNOSTICS app PRIVATE src/LocationEngine/diagnostics.c)
target_sources_ifdef(CONFIG_FINDMYCAT_DIAGNOSTICS app PRIVATE src/LocationEngine/diagnostics.h)
target_sources_ifdef(CONFIG_FINDMYCAT_DIAGNOSTICS app PRIVATE src/comm/diagnostics.h)

target_sources(app PRIVATE src/comm/nrf52_link.c)
target_sources(app PRIVATE src/comm/nrf52_link.h)
