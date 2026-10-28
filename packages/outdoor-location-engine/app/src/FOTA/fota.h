#ifndef FOTA_H
#define FOTA_H

/* Writes the update host's root certificates into the modem, which must be
 * offline. Skips the write when they are already there. */
int fota_provision_ca(void);

/* Downloads the update into MCUboot's secondary slot over HTTPS, then reboots
 * into it. The outcome is reported as the "update" command's status. */
void fota_start_update(void);

/* Reports the running firmware version once per boot; call when LTE is up.
 * After an update this is what says the new image is running. */
void fota_report_running_version(void);

#endif /* FOTA_H */
