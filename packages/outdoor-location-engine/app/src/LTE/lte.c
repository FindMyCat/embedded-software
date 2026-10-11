#include "lte.h"

#include <date_time.h>
#include <modem/lte_lc.h>
#include <nrf_modem_at.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "../DeviceStatus/device_status_reporting.h"

LOG_MODULE_REGISTER(lte, CONFIG_FINDMYCAT_LTE_LOG_LEVEL);

static K_SEM_DEFINE(lte_connected, 0, 1);

static K_SEM_DEFINE(time_update_finished, 0, 1);

static struct lte_lc_edrx_cfg granted_edrx;

/* Each attach can move the paging window, so NET waits for the first eDRX
 * wake after one and reports that moment as the window. */
static atomic_t network_report_pending;

/* Uptime the current eDRX sleep should end, 0 when not in one. An uplink
 * wakes the modem early, and that wake is no paging window. */
static int64_t scheduled_wake_uptime_ms;

#define WAKE_TOLERANCE_MS 1000

static void date_time_evt_handler(const struct date_time_evt *evt) {
  k_sem_give(&time_update_finished);
}

static void on_edrx_wake(void) {
  int64_t early_ms = scheduled_wake_uptime_ms - k_uptime_get();
  int64_t now_ms;

  scheduled_wake_uptime_ms = 0;

  if (atomic_get(&network_report_pending) == 0 ||
      early_ms > WAKE_TOLERANCE_MS) {
    return;
  }

  if (date_time_now(&now_ms) != 0) {
    LOG_WRN("No network time yet, NET report waits for the next wake");
    return;
  }

  const struct device_status status = {
      .kind = DEVICE_STATUS_NETWORK,
      .network = {.edrx = granted_edrx, .wake_ms = now_ms},
  };

  LOG_INF("Paging window opened at %lld ms (Unix), every %.2f s", now_ms,
          (double)granted_edrx.edrx);

  device_status_report(&status);
  atomic_set(&network_report_pending, 0);
}

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

    if (evt->edrx_cfg.mode != granted_edrx.mode ||
        evt->edrx_cfg.edrx != granted_edrx.edrx) {
      atomic_set(&network_report_pending, 1);
    }

    granted_edrx = evt->edrx_cfg;
    break;
  case LTE_LC_EVT_MODEM_SLEEP_ENTER:
    LOG_DBG("Modem sleep type %d for %lld ms", evt->modem_sleep.type,
            evt->modem_sleep.time);

    if (evt->modem_sleep.type == LTE_LC_MODEM_SLEEP_RF_INACTIVITY &&
        evt->modem_sleep.time > 0 && granted_edrx.mode != LTE_LC_LTE_MODE_NONE) {
      scheduled_wake_uptime_ms = k_uptime_get() + evt->modem_sleep.time;
    }
    break;
  case LTE_LC_EVT_MODEM_SLEEP_EXIT:
    if (scheduled_wake_uptime_ms != 0) {
      on_edrx_wake();
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

int lte_start(void) {
  /* A registration from an earlier attach would satisfy the next wait. */
  k_sem_reset(&lte_connected);
  atomic_set(&network_report_pending, 1);

  lte_lc_register_handler(lte_event_handler);

  /* Request eDRX before attaching; some networks reject timer changes after. */
  lte_lc_psm_req(false);

  int err = lte_lc_edrx_req(true);

  if (err) {
    LOG_INF("lte_lc_edrx_req, error: %d", err);
  }

  if (sizeof(CONFIG_FINDMYCAT_LTE_OPERATOR) > 1) {
    err = nrf_modem_at_printf("AT+COPS=1,2,\"%s\"", CONFIG_FINDMYCAT_LTE_OPERATOR);
    LOG_INF("Operator locked to %s (%d)", CONFIG_FINDMYCAT_LTE_OPERATOR, err);
  } else {
    err = nrf_modem_at_printf("AT+COPS=0");
    if (err) {
      LOG_WRN("Automatic operator selection failed (%d)", err);
    }
  }

  LOG_INF("Attaching to LTE");

  return lte_lc_connect_async(NULL);
}

int lte_stop(void) {
  LOG_INF("Powering LTE off");

  return lte_lc_power_off();
}

int lte_connect(void) {
  int err;

  if (IS_ENABLED(CONFIG_DATE_TIME)) {
    /* Registering early for date_time event handler to avoid missing
     * the first event after LTE is connected.
     */
    date_time_register_handler(date_time_evt_handler);
  }

  err = lte_start();
  if (err) {
    return err;
  }

  return lte_wait_registered(K_FOREVER);
}

int lte_wait_registered(k_timeout_t timeout) {
  return k_sem_take(&lte_connected, timeout);
}

int lte_wait_for_time(k_timeout_t timeout) {
  return k_sem_take(&time_update_finished, timeout);
}
