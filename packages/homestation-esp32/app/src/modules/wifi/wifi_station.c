#include "wifi_station.h"

#include <errno.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/net_event.h>
#include <zephyr/net/net_if.h>
#include <zephyr/net/wifi_mgmt.h>

LOG_MODULE_REGISTER(wifi_station, CONFIG_WIFI_LOG_LEVEL);

static K_SEM_DEFINE(ipv4_bound, 0, 1);
static struct net_mgmt_event_callback wifi_cb;
static struct net_mgmt_event_callback ipv4_cb;
static bool ready;

static void wifi_event_handler(struct net_mgmt_event_callback *cb,
                               uint32_t event, struct net_if *iface) {
  ARG_UNUSED(iface);

  if (event == NET_EVENT_WIFI_CONNECT_RESULT) {
    const struct wifi_status *status = (const struct wifi_status *)cb->info;

    if (status->status) {
      LOG_ERR("Error: association failed (%d)", status->status);
    } else {
      LOG_INF("Associated with %s", CONFIG_WIFI_STATION_SSID);
    }
  } else if (event == NET_EVENT_WIFI_DISCONNECT_RESULT) {
    LOG_WRN("Disconnected from %s", CONFIG_WIFI_STATION_SSID);
    ready = false;
  }
}

static void ipv4_event_handler(struct net_mgmt_event_callback *cb,
                               uint32_t event, struct net_if *iface) {
  ARG_UNUSED(cb);
  ARG_UNUSED(iface);

  if (event == NET_EVENT_IPV4_ADDR_ADD) {
    ready = true;
    k_sem_give(&ipv4_bound);
  }
}

bool wifi_station_is_ready(void) { return ready; }

int wifi_station_connect(void) {
  struct net_if *iface = net_if_get_first_wifi();

  if (!iface) {
    LOG_ERR("Error: no WiFi interface found");
    return -ENODEV;
  }

  if (strlen(CONFIG_WIFI_STATION_SSID) == 0) {
    LOG_ERR("Error: CONFIG_WIFI_STATION_SSID is empty, set it in local.conf");
    return -EINVAL;
  }

  int err = net_if_up(iface);

  if (err && err != -EALREADY) {
    LOG_ERR("Error: net_if_up failed (%d)", err);
    return err;
  }

  net_mgmt_init_event_callback(&wifi_cb, wifi_event_handler,
                               NET_EVENT_WIFI_CONNECT_RESULT |
                                   NET_EVENT_WIFI_DISCONNECT_RESULT);
  net_mgmt_add_event_callback(&wifi_cb);

  net_mgmt_init_event_callback(&ipv4_cb, ipv4_event_handler,
                               NET_EVENT_IPV4_ADDR_ADD);
  net_mgmt_add_event_callback(&ipv4_cb);

  struct wifi_connect_req_params params = {
      .ssid = (const uint8_t *)CONFIG_WIFI_STATION_SSID,
      .ssid_length = strlen(CONFIG_WIFI_STATION_SSID),
      .psk = (const uint8_t *)CONFIG_WIFI_STATION_PSK,
      .psk_length = strlen(CONFIG_WIFI_STATION_PSK),
      .security = strlen(CONFIG_WIFI_STATION_PSK) ? WIFI_SECURITY_TYPE_PSK
                                                  : WIFI_SECURITY_TYPE_NONE,
      .channel = WIFI_CHANNEL_ANY,
      .band = WIFI_FREQ_BAND_UNKNOWN,
      .mfp = WIFI_MFP_OPTIONAL,
  };

  LOG_INF("Connecting to %s", CONFIG_WIFI_STATION_SSID);

  err = net_mgmt(NET_REQUEST_WIFI_CONNECT, iface, &params, sizeof(params));

  if (err) {
    LOG_ERR("Error: connect request failed (%d)", err);
    return err;
  }

  if (k_sem_take(&ipv4_bound,
                 K_SECONDS(CONFIG_WIFI_STATION_CONNECT_TIMEOUT_S)) != 0) {
    LOG_ERR("Error: no IPv4 address within %d s",
            CONFIG_WIFI_STATION_CONNECT_TIMEOUT_S);
    return -ETIMEDOUT;
  }

  LOG_INF("WiFi station ready");
  return 0;
}
