#ifndef MESSAGE_CHANNEL_H
#define MESSAGE_CHANNEL_H

#include <zephyr/kernel.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/sys/reboot.h>
#include <zephyr/zbus/zbus.h>

#define SEND_FATAL_ERROR()								\
	do {										\
		int _unused = -1;							\
		if (zbus_chan_pub(&FATAL_ERROR_CHAN, &_unused, K_SECONDS(10))) {		\
			LOG_ERR("Fatal error channel publish failed, rebooting");	\
			LOG_PANIC();							\
			IF_ENABLED(CONFIG_REBOOT, (sys_reboot(0)));			\
		}									\
	} while (0)

ZBUS_CHAN_DECLARE(TRIGGER_CHAN, FATAL_ERROR_CHAN);

#endif /* MESSAGE_CHANNEL_H */
