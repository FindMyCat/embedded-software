#ifndef SELF_RECOVERY_H
#define SELF_RECOVERY_H

#include <zephyr/toolchain.h>

/* Records why the last boot ended and starts the watchdog */
int self_recovery_start(void);

const char *self_recovery_last_reset_cause(void);

/* Notes the reason for the next boot to report, then reboots. */
FUNC_NORETURN void self_recovery_reboot(const char *reason);

#endif /* SELF_RECOVERY_H */