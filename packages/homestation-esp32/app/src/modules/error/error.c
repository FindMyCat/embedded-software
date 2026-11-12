#include "error.h"

#include <zephyr/logging/log.h>
#include <zephyr/logging/log_ctrl.h>
#include <zephyr/sys/reboot.h>

LOG_MODULE_REGISTER(error, CONFIG_ERROR_LOG_LEVEL);

void fatal_error(void)
{
	LOG_ERR("FATAL error");

	if (IS_ENABLED(CONFIG_ERROR_REBOOT_ON_FATAL)) {
		LOG_ERR("Rebooting");
		LOG_PANIC();
		sys_reboot(SYS_REBOOT_COLD);
	}
}
