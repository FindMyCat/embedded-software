#ifndef RAK3172_H
#define RAK3172_H

#include <stddef.h>
#include <stdint.h>

int rak3172_init(void);
void rak3172_send_at(const char *cmd);
/* 0 if the module answered OK, -EIO on ERROR, -ETIMEDOUT if it went silent. */
int rak3172_read_response(int timeout_ms);
int rak3172_send_message(const char *msg);
void rak3172_suspend(void);
void rak3172_resume(void);
void rak3172_print_version_info(void);

/* Duty-cycled CAD receive. Requires the CAD firmware on the RAK3172; these
 * commands do not exist in stock RUI3. */
int rak3172_cad_rx_configure(uint8_t symbols, uint8_t det_peak, uint8_t det_min,
                             uint32_t interval_ms);
int rak3172_cad_rx_start(void);
int rak3172_cad_rx_stop(void);
int rak3172_cad_rx_read(void);
int rak3172_cad_rx_stats(void);
int rak3172_cad_rx_set_pin(uint32_t pin);
int rak3172_cad_rx_set_absence(uint32_t seconds);
int rak3172_cad_rx_event(void);
int rak3172_wait_event(const char *prefix, char *buf, size_t buf_len,
                       int timeout_ms);

#endif /* RAK3172_H */
