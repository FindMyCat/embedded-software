#ifndef RAK3172_H
#define RAK3172_H

#include <stddef.h>
#include <stdint.h>

int rak3172_init(void);
void rak3172_send_at(const char *cmd);
/* 0 if the module answered OK, -EIO on ERROR, -ETIMEDOUT if it went silent. */
int rak3172_read_response(int timeout_ms);
int rak3172_send_message(const char *msg);
int rak3172_send_payload(const uint8_t *data, size_t len);
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

/* Leading payload bytes, as hex, that a packet must match before the module
 * buffers it and wakes the host. "0" clears it. Filtered packets are still
 * detected, so they still count as presence. */
int rak3172_cad_rx_set_filter(const char *hex);

/* Leading payload bytes, as hex, that count as the peer still being alive.
 * Without this the absence timer clears on any detection, which anyone sharing
 * the band satisfies. "0" restores that. */
int rak3172_cad_rx_set_presence(const char *hex);

enum rak3172_cad_event {
  RAK3172_CAD_EVENT_NONE = 0,
  RAK3172_CAD_EVENT_PACKET,
  RAK3172_CAD_EVENT_ABSENT,
};

/* Asks the module why the wake line is where it is. The receive path reads the
 * line's shape instead, since a packet pulses and absence holds; this costs a
 * UART round trip and is for seeding the initial state, where there is no edge
 * to read. Returns an enum rak3172_cad_event, or negative on error. */
int rak3172_cad_rx_event(void);

/* Pulse the reset line. The module reports PIN in its +EVT:RESET line if the
 * line actually reached it. */
void rak3172_hw_reset(void);
int rak3172_reset_test(void);
int rak3172_wait_event(const char *prefix, char *buf, size_t buf_len,
                       int timeout_ms);

#endif /* RAK3172_H */
