#ifndef RAK3172_H
#define RAK3172_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

int rak3172_init(void);
void rak3172_send_at(const char *cmd);
int rak3172_read_response(int timeout_ms);
int rak3172_send_message(const char *msg);
int rak3172_send_payload(const uint8_t *data, size_t len);

int rak3172_sleep(void);
int rak3172_low_power(bool enable);

void rak3172_print_version_info(void);

void rak3172_uart_claim(void);
void rak3172_uart_release(void);

int rak3172_cad_rx_configure(uint8_t symbols, uint8_t det_peak, uint8_t det_min,
                             uint32_t interval_ms, uint8_t max_payload,
                             uint16_t peer_preamble);
int rak3172_cad_rx_start(void);
int rak3172_cad_rx_stop(void);
int rak3172_cad_rx_read(void);
int rak3172_cad_rx_stats(void);
int rak3172_cad_rx_set_pin(uint32_t pin);
int rak3172_cad_rx_set_absence(uint32_t seconds);
int rak3172_cad_rx_set_duty(uint32_t burst_ms, uint32_t period_ms);
/* Module-sent uplink at a CAD burst start every period_s; 0 turns it off. */
int rak3172_set_heartbeat(uint32_t period_s, const char *hex);

int rak3172_cad_rx_set_filter(const char *hex);
int rak3172_cad_rx_set_presence(const char *hex);

enum rak3172_cad_event {
  RAK3172_CAD_EVENT_NONE = 0,
  RAK3172_CAD_EVENT_PACKET,
  RAK3172_CAD_EVENT_ABSENT,
};

int rak3172_cad_rx_event(void);

void rak3172_hw_reset(void);
int rak3172_wait_event(const char *prefix, char *buf, size_t buf_len,
                       int timeout_ms);

#endif /* RAK3172_H */
