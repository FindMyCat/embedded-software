#include <modem/sms.h>
#ifndef SMS_LISTENER_DOT_H    /* This is an "include guard" */
#define SMS_LISTENER_DOT_H    /* prevents the file from being included twice. */
                              /* Including a header file twice causes all kinds */
                              /* of interesting problems.*/

void sms_listener_init(void* smsCallback);

#endif
