#ifndef ERROR_H
#define ERROR_H

/* Logs the failure and, with CONFIG_ERROR_REBOOT_ON_FATAL, reboots. Returns
 * only when rebooting is off. */
void fatal_error(void);

#endif /* ERROR_H */
