menu "Trigger"

config TRIGGER_THREAD_STACK_SIZE
	int "Thread stack size"
	default 512

config TRIGGER_TIMEOUT_SECONDS
	int "Trigger interval in seconds"
	default 30

module = TRIGGER
module-str = Trigger
source "subsys/logging/Kconfig.template.log_config"

endmenu # Trigger
