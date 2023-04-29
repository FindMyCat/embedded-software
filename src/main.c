/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

// Libraries
#include <string.h>
#include <zephyr/kernel.h>
#include <nrf_modem_at.h>
#include <modem/lte_lc.h>
#include <modem/location.h>
#include <date_time.h>
#include <zephyr/logging/log.h>

// Application specific imports
#include "location.h"
#include "./mqtt/mqtt.h"

LOG_MODULE_REGISTER(main, 4);

static K_SEM_DEFINE(lte_connected, 0, 1);

static K_SEM_DEFINE(time_update_finished, 0, 1);

static void date_time_evt_handler(const struct date_time_evt *evt)
{
	k_sem_give(&time_update_finished);
}

static void lte_event_handler(const struct lte_lc_evt *const evt)
{
	switch (evt->type) {
	case LTE_LC_EVT_NW_REG_STATUS:
		if ((evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_HOME) ||
		     (evt->nw_reg_status == LTE_LC_NW_REG_REGISTERED_ROAMING)) {
			LOG_INF("Connected to LTE\n");
			k_sem_give(&lte_connected);
		}
		break;
	default:
		break;
	}
}


int main(void)
{
	int err;

	if (IS_ENABLED(CONFIG_DATE_TIME)) {
		/* Registering early for date_time event handler to avoid missing
		 * the first event after LTE is connected.
		 */
		date_time_register_handler(date_time_evt_handler);
	}

	LOG_INF("Connecting to LTE...\n");

	lte_lc_init();
	lte_lc_register_handler(lte_event_handler);

	/* Enable PSM. */
	/* Turn on LTE power saving features. Also,
	 * request power saving features before network registration. Some
	 * networks rejects timer updates after the device has registered to the
	 * LTE network.
	 */
	// lte_lc_psm_req(true);
	/** enhanced Discontinuous Reception */

	LOG_INF("Setting EDRX mode");
	err = lte_lc_edrx_req(true);
	
	if (err) {
		LOG_INF("lte_lc_edrx_req, error: %d\n", err);
	}

	lte_lc_connect();

	k_sem_take(&lte_connected, K_FOREVER);

	/* A-GPS/P-GPS needs to know the current time. */
	if (IS_ENABLED(CONFIG_DATE_TIME)) {
		LOG_INF("Waiting for current time\n");

		/* Wait for an event from the Date Time library. */
		k_sem_take(&time_update_finished, K_MINUTES(10));

		if (!date_time_is_valid()) {
			LOG_INF("Failed to get current time. Continuing anyway.\n");
		}
	}

	err = location_init(location_event_handler);
	if (err) {
		LOG_INF("Initializing the Location library failed, error: %d\n", err);
		return -1;
	}

	location_gnss_high_accuracy_get();	

	// mqtt_main();

	return 0;
}
