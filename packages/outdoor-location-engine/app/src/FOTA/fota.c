#include "fota.h"

#include <app_version.h>
#include <modem/lte_lc.h>
#include <modem/modem_key_mgmt.h>
#include <net/fota_download.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/reboot.h>

#include "../DeviceStatus/device_status_reporting.h"
#include "../Presence/presence.h"

LOG_MODULE_REGISTER(fota, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

#define FOTA_SEC_TAG 42

/* Long enough for the "installed" status to leave over MQTT-SN first. */
#define REBOOT_DELAY K_SECONDS(15)

static const char ca_roots[] = {
#include "isrg_roots.pem.inc"
};

int fota_provision_ca(void) {
  if (modem_key_mgmt_cmp(FOTA_SEC_TAG, MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN,
                         ca_roots, strlen(ca_roots)) == 0) {
    return 0;
  }

  int err = modem_key_mgmt_write(FOTA_SEC_TAG, MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN,
                                 ca_roots, strlen(ca_roots));

  LOG_INF("Update root certificates written to sec tag %d (%d)", FOTA_SEC_TAG,
          err);
  return err;
}

/* fota_download has already marked the new image for a trial boot. */
static void reboot_work_handler(struct k_work *work) {
  ARG_UNUSED(work);

  lte_lc_power_off();
  sys_reboot(SYS_REBOOT_WARM);
}

static K_WORK_DELAYABLE_DEFINE(reboot_work, reboot_work_handler);

/* Quarters already reported for this download, so 25/50/75% go out once. */
static int reported_quarters;

static void report_progress(int percent) {
  char reason[8];

  snprintk(reason, sizeof(reason), "%d%%", percent);
  device_status_report_command("update", "downloading", reason);
}

static void fota_event_handler(const struct fota_download_evt *evt) {
  switch (evt->id) {
  case FOTA_DOWNLOAD_EVT_PROGRESS:
    LOG_INF("Update download %d%%", evt->progress);
    if (evt->progress / 25 > reported_quarters && evt->progress < 100) {
      reported_quarters = evt->progress / 25;
      report_progress(reported_quarters * 25);
    }
    break;
  case FOTA_DOWNLOAD_EVT_FINISHED:
    LOG_INF("Update downloaded, rebooting into it");
    device_status_report_command("update", "ok", "installed, rebooting");
    k_work_schedule(&reboot_work, REBOOT_DELAY);
    break;
  case FOTA_DOWNLOAD_EVT_ERROR:
    LOG_ERR("Error: update download failed (cause %d)", evt->cause);
    device_status_report_command("update", "fail", "download failed");
    break;
  case FOTA_DOWNLOAD_EVT_CANCELLED:
    device_status_report_command("update", "fail", "download cancelled");
    break;
  default:
    break;
  }
}

void fota_start_update(void) {
  static bool initialized;

  /* Status only reaches the cloud over LTE, so neither would the download. */
  if (presence_link_get() != PRESENCE_LINK_LTE) {
    LOG_WRN("Update needs LTE as the link, ignoring");
    return;
  }

  if (!initialized) {
    int err = fota_download_init(fota_event_handler);

    if (err) {
      LOG_ERR("Error: fota_download_init failed (%d)", err);
      device_status_report_command("update", "fail", "init failed");
      return;
    }
    initialized = true;
  }

  LOG_INF("Downloading update from %s/%s", CONFIG_FINDMYCAT_FOTA_HOST,
          CONFIG_FINDMYCAT_FOTA_FILE);

  int err = fota_download_start(CONFIG_FINDMYCAT_FOTA_HOST,
                                CONFIG_FINDMYCAT_FOTA_FILE, FOTA_SEC_TAG, 0, 0);

  if (err) {
    LOG_ERR("Error: fota_download_start failed (%d)", err);
    device_status_report_command("update", "fail",
                                 err == -EALREADY ? "already downloading"
                                                  : "start failed");
    return;
  }

  reported_quarters = 0;
  report_progress(0);
}

void fota_report_running_version(void) {
  static bool reported;

  if (!reported) {
    device_status_report_firmware(APP_VERSION_STRING);
    reported = true;
  }
}
