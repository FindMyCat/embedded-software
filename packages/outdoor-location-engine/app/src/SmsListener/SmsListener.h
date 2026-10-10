#ifndef SMS_LISTENER_DOT_H
#define SMS_LISTENER_DOT_H

/* Call once LTE has registered; the SMS library re-subscribes on later
 * attaches by itself. Repeat calls do nothing. */
int sms_listener_start(void);

#endif
