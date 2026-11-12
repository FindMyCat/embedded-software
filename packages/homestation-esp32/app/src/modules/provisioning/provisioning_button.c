#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "provisioning.h"

LOG_MODULE_REGISTER(provisioning_button, LOG_LEVEL_INF);

#define HOLD_FOR_PROVISIONING_MS (5 * MSEC_PER_SEC)

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET_OR(DT_ALIAS(sw0), gpios, {0});
static struct gpio_callback button_callback;

static void open_provisioning_after_hold(struct k_work *work)
{
	ARG_UNUSED(work);
	provisioning_window_open();
}

static K_WORK_DELAYABLE_DEFINE(hold_work, open_provisioning_after_hold);

static void button_changed(const struct device *port, struct gpio_callback *callback,
			   gpio_port_pins_t pins)
{
	ARG_UNUSED(port);
	ARG_UNUSED(callback);
	ARG_UNUSED(pins);

	if (gpio_pin_get_dt(&button) > 0) {
		k_work_reschedule(&hold_work, K_MSEC(HOLD_FOR_PROVISIONING_MS));
	} else {
		k_work_cancel_delayable(&hold_work);
	}
}

void provisioning_button_start(void)
{
	if (!gpio_is_ready_dt(&button)) {
		LOG_WRN("No button wired; provisioning opens only while unprovisioned");
		return;
	}

	if (gpio_pin_configure_dt(&button, GPIO_INPUT) != 0 ||
	    gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_BOTH) != 0) {
		LOG_ERR("Error: configuring the button failed");
		return;
	}

	gpio_init_callback(&button_callback, button_changed, BIT(button.pin));
	gpio_add_callback(button.port, &button_callback);
	LOG_INF("Button ready: hold %d s to provision", HOLD_FOR_PROVISIONING_MS / MSEC_PER_SEC);
}