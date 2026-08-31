#ifndef RAK3172_H
#define RAK3172_H

#include <stddef.h>
#include <stdint.h>

int rak3172_init(void);
void rak3172_send_at(const char *cmd);
void rak3172_read_response(int timeout_ms);
int rak3172_send_message(const char *msg);
/* Binary-safe form of rak3172_send_message, for binary frames. */
int rak3172_send_payload(const uint8_t *payload, size_t len);
void rak3172_suspend(void);
void rak3172_resume(void);
void rak3172_print_version_info(void);
void rak3172_set_preamble(uint16_t symbols);
void rak3172_start_rx(void);
int rak3172_read_event(char *buf, size_t buf_len, int timeout_ms);

#endif /* RAK3172_H */
