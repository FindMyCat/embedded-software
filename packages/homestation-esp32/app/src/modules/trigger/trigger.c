#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>

#include "message_channel.h"

LOG_MODULE_REGISTER(trigger, CONFIG_TRIGGER_LOG_LEVEL);

static void trigger_task(void)
{
	while (true) {
		int not_used = -1;
		int err = zbus_chan_pub(&TRIGGER_CHAN, &not_used, K_SECONDS(1));

		if (err) {
			LOG_ERR("Error: zbus_chan_pub failed (%d)", err);
			SEND_FATAL_ERROR();
		}

		LOG_INF("Trigger fired");
		k_sleep(K_SECONDS(CONFIG_TRIGGER_TIMEOUT_SECONDS));
	}
}

K_THREAD_DEFINE(trigger_task_id,
		CONFIG_TRIGGER_THREAD_STACK_SIZE,
		trigger_task, NULL, NULL, NULL, 3, 0, 0);
