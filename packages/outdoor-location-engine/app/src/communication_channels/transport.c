#include "transport.h"

#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "../FuelGauge/fuel_gauge.h"
#include "../LoRa/lora_link.h"
#include "../Presence/presence.h"
#include "../mqtt/mqttsn.h"

LOG_MODULE_REGISTER(transport,
                    CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

#define POSITION_JSON_BYTES 100


static uint16_t get_accuracy_decimetres(double accuracy_metres) {
  double decimetres = accuracy_metres * 10.0;

  if (accuracy_metres < 0.0 || decimetres >= (double)UINT16_MAX) {
    return UINT16_MAX;
  }

  return (uint16_t)decimetres;
}

#if defined(CONFIG_LORA_LINK)
static int publish_over_lora(const struct position_report *report) {
  // encodes position_report into LORA frame
  struct lora_frame_position position = {
      .latitude_microdegrees = (int32_t)(report->latitude * 1000000.0),
      .longitude_microdegrees = (int32_t)(report->longitude * 1000000.0),
      .accuracy_decimetres = get_accuracy_decimetres(report->accuracy_metres),
      .satellites = (uint8_t)report->satellites,
      .battery_percent = fuel_gauge_battery_percent(),
  };

  return lora_link_send_position(&position);
}
#endif /* CONFIG_LORA_LINK */

static int publish_over_cellular(const struct position_report *report) {
  char json[POSITION_JSON_BYTES];

  snprintk(json, sizeof(json), "{\"msg\": \"%.06f,%.06f,%d,%.02f,%d\"}",
           report->latitude, report->longitude, report->satellites,
           report->accuracy_metres, fuel_gauge_battery_percent());

  return mqttsn_publish_with_session(CONFIG_MQTT_SN_LOCATION_TOPIC, json);
}

int transport_publish_position_via(const struct position_report *report,
                                   enum transport_path path) {
#if defined(CONFIG_LORA_LINK)
  if (path == TRANSPORT_PATH_LORA) {
    int err = publish_over_lora(report);

    if (err == 0) {
      LOG_INF("Position sent over LoRa");
    }

    /* No fallback: the caller asked for this link specifically. */
    return err;
  }

  if (path == TRANSPORT_PATH_AUTO && lora_link_homestation_in_range()) {
    int err = publish_over_lora(report);

    if (err == 0) {
      LOG_INF("Position sent over LoRa");
      return 0;
    }

    LOG_WRN("LoRa send failed (%d), falling back to MQTT-SN", err);
  }
#else
  if (path == TRANSPORT_PATH_LORA) {
    LOG_ERR("Error: LoRa asked for but CONFIG_LORA_LINK is off");
    return -ENOTSUP;
  }
#endif

  return publish_over_cellular(report);
}

int transport_publish_position(const struct position_report *report) {
  return transport_publish_position_via(report, TRANSPORT_PATH_AUTO);
}

/* The LoRa frame's bytes, hex encoded: the MQTT-SN client takes a string, and
 * the frame has zero bytes in it.
 * TODO: evaluate sending it binary; hex doubles the frame (~49 bytes a ping).
 * Needs a length-taking publish in mqttsn.c and bin2hexstr(payload) in the
 * EMQX rule. */
static int publish_wifi_scan_over_cellular(const struct wifi_scan_frame *scan) {
  char hex[2 * sizeof(scan->bytes) + 1];

  for (size_t i = 0; i < scan->len; i++) {
    snprintk(&hex[2 * i], 3, "%02X", scan->bytes[i]);
  }
  hex[2 * scan->len] = '\0';

  return mqttsn_publish_with_session(CONFIG_MQTT_SN_WIFI_SCAN_TOPIC, hex);
}

int transport_publish_wifi_scan(const struct wifi_scan_frame *scan) {
  /* The home station resolves the scan over LoRa, the server over LTE. */
  if (presence_link_get() == PRESENCE_LINK_LTE) {
    return publish_wifi_scan_over_cellular(scan);
  }

#if defined(CONFIG_LORA_LINK)
  return lora_link_send_wifi_scan(scan);
#else
  return publish_wifi_scan_over_cellular(scan);
#endif
}
