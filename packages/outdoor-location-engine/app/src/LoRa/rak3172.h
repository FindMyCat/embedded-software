#ifndef RAK3172_H
#define RAK3172_H

int rak3172_init(void);
void rak3172_send_at(const char *cmd);
void rak3172_read_response(int timeout_ms);
int rak3172_send_message(const char *msg);
void rak3172_suspend(void);
void rak3172_resume(void);
void rak3172_print_version_info(void);

#endif /* RAK3172_H */
