#ifndef DEVICE_STATUS_REPORTING_H
#define DEVICE_STATUS_REPORTING_H

#include <modem/lte_lc.h>

enum device_status_kind {
  DEVICE_STATUS_COMMAND,
  DEVICE_STATUS_NETWORK,
};

/* One report for the device status topic. The JSON on the wire stays flat,
 * so the server rule and the app read every kind the same way. */
struct device_status {
  enum device_status_kind kind;
  union {
    struct {
      char command[16];
      char status[8];
      char reason[32];
    } command;
    struct lte_lc_edrx_cfg network;
  };
};

/* Copies the status into a queue and returns; publishing happens on the
 * reporting thread, so this is safe from event handlers. */
void device_status_report(const struct device_status *status);

#endif /* DEVICE_STATUS_REPORTING_H */
