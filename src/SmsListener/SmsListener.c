#include <stdio.h>
#include <zephyr/kernel.h>
#include <string.h>
#include <modem/sms.h>


// LOG_MODULE_REGISTER(main, 4);
void sms_listener_init(void* smsCallback) {
    int handle = 0;

	printk("\nSMS sample starting\n");

	handle = sms_register_listener(smsCallback, NULL);
	if (handle) {
		printk("sms_register_listener returned err: %d\n", handle);
		return;
	}

	printk("SMS sample is ready for receiving messages\n");


	/* In our application, we should unregister SMS in some conditions with:
	 *   sms_unregister_listener(handle);
	 * However, this sample will continue to be registered for
	 * received SMS messages and they can be seen in serial port log.
	 */
}