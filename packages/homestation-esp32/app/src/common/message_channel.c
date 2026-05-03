#include <zephyr/zbus/zbus.h>

#include "message_channel.h"

ZBUS_CHAN_DEFINE(TRIGGER_CHAN,
		int,
		NULL,
		NULL,
		ZBUS_OBSERVERS_EMPTY,
		ZBUS_MSG_INIT(0)
);

ZBUS_CHAN_DEFINE(FATAL_ERROR_CHAN,
		int,
		NULL,
		NULL,
		ZBUS_OBSERVERS(error),
		ZBUS_MSG_INIT(0)
);
