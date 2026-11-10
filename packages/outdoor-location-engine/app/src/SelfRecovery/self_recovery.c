#include "self_recovery.h"

#include <modem/lte_lc.h>
#include <nrf_modem.h>
#include <string.h>
#include <zephyr/drivers/hwinfo.h>
#include <zephyr/drivers/watchdog.h>
#include <zephyr/fatal.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/sys/reboot.h>

LOG_MODULE_REGISTER(self_recovery,
                    CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

/* Fed from the system workqueue, so a blocked queue (nRF52 link, power mode)
 * resets the chip instead of leaving it deaf. */
#define WATCHDOG_TIMEOUT_MS (5 * 60 * MSEC_PER_SEC)
#define WATCHDOG_FEED_PERIOD K_SECONDS(60)

#define REBOOT_NOTE_MAGIC 0x524E4F54
#define RESET_CAUSE_LENGTH 24

/* RAM keeps this across a reboot */
static __noinit struct {
  uint32_t magic;
  char reason[RESET_CAUSE_LENGTH];
} reboot_note;

static char last_reset_cause[RESET_CAUSE_LENGTH] = "unknown";

static const struct device *const watchdog = DEVICE_DT_GET(DT_NODELABEL(wdt0));
static int watchdog_channel;

static void feed_watchdog(struct k_work *work);
static K_WORK_DELAYABLE_DEFINE(feed_watchdog_work, feed_watchdog);

static void feed_watchdog(struct k_work *work) {
  ARG_UNUSED(work);
  wdt_feed(watchdog, watchdog_channel);
  k_work_schedule(&feed_watchdog_work, WATCHDOG_FEED_PERIOD);
}

FUNC_NORETURN void self_recovery_reboot(const char *reason) {
  /* Detach so the carrier ends the data session; otherwise inbound UDP keeps
   * routing to the dead one for hours. Not possible from a fault or interrupt. */
  if (!k_is_in_isr()) {
    (void)lte_lc_power_off();
  }

  reboot_note.magic = REBOOT_NOTE_MAGIC;
  strncpy(reboot_note.reason, reason, sizeof(reboot_note.reason) - 1);
  reboot_note.reason[sizeof(reboot_note.reason) - 1] = '\0';
  sys_reboot(SYS_REBOOT_COLD);
}

/* Replaces the kernel's halt, which left the chip dead until power cycled. */
void k_sys_fatal_error_handler(unsigned int reason, const struct arch_esf *esf) {
  char note[RESET_CAUSE_LENGTH];

  LOG_PANIC();
  /* PC of the faulting instruction; addr2line on this build's zephyr.elf names
   * the line. Some fatal paths pass no frame. */
  if (esf != NULL) {
    snprintk(note, sizeof(note), "fatal %u pc 0x%08x", reason,
             (unsigned int)esf->basic.pc);
  } else {
    snprintk(note, sizeof(note), "fatal %u", reason);
  }
  self_recovery_reboot(note);
}

/* Runs in the IPC interrupt. A full reboot brings app and modem back together,
 * where a modem-only reset would leave LTE state stale. */
void nrf_modem_fault_handler(struct nrf_modem_fault_info *fault) {
  char note[RESET_CAUSE_LENGTH];

  snprintk(note, sizeof(note), "modem fault 0x%x", fault->reason);
  self_recovery_reboot(note);
}

static const char *hardware_reset_cause(void) {
  uint32_t cause;

  if (hwinfo_get_reset_cause(&cause) != 0) {
    return "unknown";
  }

  /* The register is sticky; clear it so the next boot reads only its own. */
  (void)hwinfo_clear_reset_cause();

  if (cause & RESET_WATCHDOG) {
    return "watchdog";
  }
  if (cause & RESET_CPU_LOCKUP) {
    return "cpu lockup";
  }
  if (cause & RESET_SOFTWARE) {
    return "software reset";
  }
  if (cause & RESET_PIN) {
    return "reset pin";
  }
  if (cause & RESET_LOW_POWER_WAKE) {
    return "low power wake";
  }
  if (cause & RESET_DEBUG) {
    return "debugger reset";
  }
  return cause == 0 ? "power on" : "other";
}

const char *self_recovery_last_reset_cause(void) { return last_reset_cause; }

int self_recovery_start(void) {
  const char *cause = hardware_reset_cause();

  if (reboot_note.magic == REBOOT_NOTE_MAGIC) {
    cause = reboot_note.reason;
  }
  strncpy(last_reset_cause, cause, sizeof(last_reset_cause) - 1);
  reboot_note.magic = 0;
  LOG_INF("Last boot ended by: %s", last_reset_cause);

  if (!device_is_ready(watchdog)) {
    LOG_ERR("Error: watchdog not ready, hangs will not reset");
    return -ENODEV;
  }

  const struct wdt_timeout_cfg config = {
      .window.max = WATCHDOG_TIMEOUT_MS,
      .flags = WDT_FLAG_RESET_SOC,
  };

  watchdog_channel = wdt_install_timeout(watchdog, &config);
  if (watchdog_channel < 0) {
    LOG_ERR("Error: watchdog timeout install failed (%d)", watchdog_channel);
    return watchdog_channel;
  }

  int err = wdt_setup(watchdog, WDT_OPT_PAUSE_HALTED_BY_DBG);
  if (err) {
    LOG_ERR("Error: watchdog setup failed (%d)", err);
    return err;
  }

  k_work_schedule(&feed_watchdog_work, WATCHDOG_FEED_PERIOD);
  return 0;
}