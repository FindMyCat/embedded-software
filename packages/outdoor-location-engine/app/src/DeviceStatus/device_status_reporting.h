#ifndef DEVICE_STATUS_REPORTING_H
#define DEVICE_STATUS_REPORTING_H

#include <modem/lte_lc.h>

enum device_status_kind {
  DEVICE_STATUS_COMMAND,
  DEVICE_STATUS_NETWORK,
  DEVICE_STATUS_FIRMWARE,
};

/* One report for the device status topic. The JSON on the wire stays flat,
 * so the server rule and the app read every kind the same way. */
struct device_status {
  enum device_status_kind kind;
  union {
    struct {
      char command[16];
      char status[16];
      char reason[32];
    } command;
    struct lte_lc_edrx_cfg network;
    /* Version of the running image, e.g. "3.0.1". */
    char firmware[16];
  };
};

/* Copies the status into a queue and returns; publishing happens on the
 * reporting thread, so this is safe from event handlers. */
void device_status_report(const struct device_status *status);

/* A command's outcome, e.g. ("update", "fail", "download error"). */
void device_status_report_command(const char *command, const char *status,
                                  const char *reason);

void device_status_report_firmware(const char *version);

#endif /* DEVICE_STATUS_REPORTING_H */
