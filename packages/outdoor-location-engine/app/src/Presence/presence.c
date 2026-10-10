#include "presence.h"

#include <zephyr/kernel.h>

#include "../Link/link_orchestrator.h"

/* Set from the power-mode work and the LoRa thread. */
static K_MUTEX_DEFINE(presence_lock);

static bool ble_in_range;
/* LoRa counts as present until the module says otherwise; see below. */
static bool lora_in_range = true;
static enum presence_link link = PRESENCE_LINK_LORA;

/* Notifies on every report, not only changes, so the boot report sets the
 * radios too. */
static void update_link(void) {
  if (IS_ENABLED(CONFIG_FINDMYCAT_FORCE_LTE_LINK)) {
    link = PRESENCE_LINK_LTE;
  } else if (ble_in_range) {
    link = PRESENCE_LINK_BLE;
  } else if (IS_ENABLED(CONFIG_LORA_LINK) && lora_in_range) {
    link = PRESENCE_LINK_LORA;
  } else {
    link = PRESENCE_LINK_LTE;
  }

  link_orchestrator_notify();
}

void presence_set_ble(bool in_range) {
  k_mutex_lock(&presence_lock, K_FOREVER);

  /* Leaving BLE restarts CAD, and the module flags absence only after
   * CONFIG_LORA_LINK_ABSENCE_S of silence, so LoRa counts as present till
   * then. */
  if (ble_in_range && !in_range) {
    lora_in_range = true;
  }

  ble_in_range = in_range;
  update_link();
  k_mutex_unlock(&presence_lock);
}

void presence_set_lora(bool in_range) {
  k_mutex_lock(&presence_lock, K_FOREVER);
  lora_in_range = in_range;
  update_link();
  k_mutex_unlock(&presence_lock);
}

enum presence_link presence_link_get(void) { return link; }

const char *presence_link_name(enum presence_link which) {
  switch (which) {
  case PRESENCE_LINK_BLE:
    return "BLE";
  case PRESENCE_LINK_LORA:
    return "LoRa";
  case PRESENCE_LINK_LTE:
    return "LTE";
  }

  return "?";
}
