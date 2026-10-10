#include "link_orchestrator.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "../LTE/lte.h"
#include "../LoRa/lora_link.h"
#include "../Presence/presence.h"
#include "../SmsListener/SmsListener.h"
#include "../UDPListener/UDPListener.h"

LOG_MODULE_REGISTER(link_orchestrator,
                    CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

#define ORCHESTRATOR_STACK_SIZE 2048
#define ORCHESTRATOR_PRIORITY 7
#define LTE_REGISTER_POLL K_SECONDS(5)

static K_SEM_DEFINE(link_changed, 0, 1);

static bool lte_up;
static bool udp_up;

void link_orchestrator_notify(void) { k_sem_give(&link_changed); }

/* Attach blocks until registration, so it waits here; gives up and powers the
 * modem off if the home station comes back first. */
static void bring_lte_up(void) {
  int err = lte_start();

  if (err != 0) {
    LOG_ERR("Error: LTE attach failed to start (%d)", err);
    return;
  }

  while (lte_wait_registered(LTE_REGISTER_POLL) != 0) {
    if (presence_link_get() != PRESENCE_LINK_LTE) {
      (void)lte_stop();
      return;
    }
  }

  lte_up = true;

  (void)sms_listener_start();

  err = create_udp_listener();
  if (err != 0) {
    LOG_ERR("Error: UDP listener failed (%d), pings will not arrive", err);
    return;
  }

  start_udp_listener_thread();
  udp_up = true;
}

static void bring_lte_down(void) {
  if (udp_up) {
    stop_udp_listener_thread();
    (void)destroy_udp_listener();
    udp_up = false;
  }

  (void)lte_stop();
  lte_up = false;
}

static void orchestrate(void *unused1, void *unused2, void *unused3) {
  ARG_UNUSED(unused1);
  ARG_UNUSED(unused2);
  ARG_UNUSED(unused3);

  bool first = true;
  enum presence_link last;

  while (true) {
    k_sem_take(&link_changed, K_FOREVER);

    enum presence_link link = presence_link_get();

    if (first) {
      LOG_INF("Link = %s", presence_link_name(link));
    } else if (link != last) {
      LOG_INF("Link %s -> %s", presence_link_name(last),
              presence_link_name(link));
    }

    first = false;
    last = link;

#if defined(CONFIG_LORA_LINK)
    /* LoRa keeps listening under LTE to hear the home station return, unless
     * LTE is forced and nothing would act on hearing it. */
    lora_link_set_enabled(link != PRESENCE_LINK_BLE &&
                          !IS_ENABLED(CONFIG_FINDMYCAT_FORCE_LTE_LINK));
#endif

    bool want_lte = link == PRESENCE_LINK_LTE;

    if (want_lte && !lte_up) {
      bring_lte_up();
    } else if (!want_lte && lte_up) {
      bring_lte_down();
    }
  }
}

K_THREAD_DEFINE(link_orchestrator_thread, ORCHESTRATOR_STACK_SIZE, orchestrate,
                NULL, NULL, NULL, ORCHESTRATOR_PRIORITY, 0, 0);
