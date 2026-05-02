#ifndef RAK3172_H
#define RAK3172_H

int rak3172_init(void);
void rak3172_send_at(const char *cmd);
void rak3172_read_response(int timeout_ms);
int rak3172_send_message(const char *msg);

#endif /* RAK3172_H */
