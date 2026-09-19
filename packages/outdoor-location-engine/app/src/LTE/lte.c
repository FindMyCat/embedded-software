#include "lte.h"

#include <date_time.h>
#include <modem/lte_lc.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "../Responder/Responder.h"

LOG_MODULE_REGISTER(lte, CONFIG_FINDMYCAT_LTE_LOG_LEVEL);

static K_SEM_DEFINE(lte_connected, 0, 1);

static K_SEM_DEFINE(time_update_finished, 0, 1);

static struct lte_lc_edrx_cfg published_edrx;
static struct lte_lc_edrx_cfg pending_edrx;

static void date_time_evt_handler(const struct date_time_evt *evt) {
  k_sem_give(&time_update_finished);
}

static void edrx_publish_work_handler(struct k_work *work) {
  struct lte_lc_edrx_cfg cfg = pending_edrx;

  publish_network_status_to_device_status_topic(&cfg);
  published_edrx = cfg;
}

static K_WORK_DEFINE(edrx_publish_work, edrx_publish_work_handler);

static void lte_event_handler(const struct lte_lc_evt *const evt) {
  switch (evt->type) {
  case LTE_LC_EVT_NW_REG_STATUS:
    if ((evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME) ||
        (evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_ROAMING)) {
      LOG_INF("Connected to LTE");
      k_sem_give(&lte_connected);
    }
    break;
  case LTE_LC_EVT_EDRX_UPDATE:
    LOG_INF("eDRX granted: mode %d, interval %.2f s, PTW %.2f s",
            evt->edrx_cfg.mode, (double)evt->edrx_cfg.edrx,
            (double)evt->edrx_cfg.ptw);

    if (evt->edrx_cfg.mode != published_edrx.mode ||
        evt->edrx_cfg.edrx != published_edrx.edrx) {
      pending_edrx = evt->edrx_cfg;
      k_work_submit(&edrx_publish_work);
    }
    break;
  case LTE_LC_EVT_PSM_UPDATE:
    LOG_INF("PSM granted: TAU %d s, active time %d s", evt->psm_cfg.tau,
            evt->psm_cfg.active_time);
    break;
  case LTE_LC_EVT_RRC_UPDATE:
    LOG_INF("RRC mode: %s", evt->rrc_mode == LTE_LC_RRC_MODE_CONNECTED
                                ? "connected"
                                : "idle");
    break;
  case LTE_LC_EVT_RAI_UPDATE:
    LOG_DBG("RAI on cell 0x%08x: AS %s, CP %s", evt->rai_cfg.cell_id,
            evt->rai_cfg.as_rai ? "yes" : "no",
            evt->rai_cfg.cp_rai ? "yes" : "no");
    break;
  default:
    LOG_DBG("Unhandled LTE event %d", evt->type);
    break;
  }
}

int lte_connect(void) {
  int err;

  if (IS_ENABLED(CONFIG_DATE_TIME)) {
    /* Registering early for date_time event handler to avoid missing
     * the first event after LTE is connected.
     */
    date_time_register_handler(date_time_evt_handler);
  }

  LOG_INF("Connecting to LTE...");

  lte_lc_register_handler(lte_event_handler);

  /* Enable PSM. */
  /* Turn on LTE power saving features. Also,
   * request power saving features before network registration. Some
   * networks rejects timer updates after the device has registered to the
   * LTE network.
   */
  lte_lc_psm_req(false);

  LOG_INF("Requesting EDRX mode");

  err = lte_lc_edrx_req(true);

  if (err) {
    LOG_INF("lte_lc_edrx_req, error: %d", err);
  }

  err = lte_lc_connect();

  k_sem_take(&lte_connected, K_FOREVER);

  return err;
}

int lte_wait_for_time(k_timeout_t timeout) {
  return k_sem_take(&time_update_finished, timeout);
}
