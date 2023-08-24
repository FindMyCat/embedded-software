/*
 * Copyright (c) 2020–2022 Qorvo, Inc
 *
 * All rights reserved.
 *
 * NOTICE: All information contained herein is, and remains the property
 * of Qorvo, Inc. and its suppliers, if any. The intellectual and technical
 * concepts herein are proprietary to Qorvo, Inc. and its suppliers, and
 * may be covered by patents, patent applications, and are protected by
 * trade secret and/or copyright law. Dissemination of this information
 * or reproduction of this material is strictly forbidden unless prior written
 * permission is obtained from Qorvo, Inc.
 *
 */

#ifndef UCI_BACKEND_FIRA_H
#define UCI_BACKEND_FIRA_H

#include <uci/uci.h>
#include <uci_backend/uci_backend_coordinator.h>
#include <uci_backend/uci_backend_manager.h>
#include <uwbmac/fira_helper.h>
#include <uwbmac/uwbmac.h>

/**
 * DOC: Backend FiRa overview
 *
 * The uci_backend_fira implements the control of ranging type session.
 *
 * For every UCI commands relative to a session, the backend manager will dispatch it to this fira backend if
 * the session is of type ranging. Then this fira backend will realise the command through uwbmac API using the fira helper interface.
 * This will then be realized by the fira region inside the MAC.
 *
 */

/**
 * enum meas_seq_template_type - Measurement step type.
 *
 * @TEMPLATE_RANGING:
 *     Ranging measure.
 * @TEMPLATE_COMBO:
 *     Both azimuth and elevation measure.
 * @TEMPLATE_AZIMUTH:
 *     Azimuth measure.
 * @TEMPLATE_ELEVATION:
 *     Elevation measure.
 * @TEMPLATE_NB:
 *     Internal use.
 */
enum meas_seq_template_type {
	TEMPLATE_RANGING = 0,
	TEMPLATE_COMBO = 1,
	TEMPLATE_AZIMUTH = 2,
	TEMPLATE_ELEVATION = 3,
	TEMPLATE_NB
};

/**
 * struct antenna_parameters - Definition of antennas parameters.
 */
struct antenna_parameters {
	/**
	 * @ch5: Fira measurement sequence step for channel 5.
	 */
	struct measurement_sequence_step ch5[TEMPLATE_NB];
	/**
	 * @ch9: Fira measurement sequence step for channel 9.
	 */
	struct measurement_sequence_step ch9[TEMPLATE_NB];
	/**
	 * @aoa_capability: Angle Of Arrival capability: 0 (no AOA), 1 (Azimuth only), 2 (Azimuth and Elevation).
	 */
	uint8_t aoa_capability;
};

/**
 * struct stop_ntf_policy - Definition of stop notification policy.
 */
struct stop_ntf_policy {
	/**
 	* @session_id: the session being stopped, or negative value if no session are being stopped.
	*/
	int64_t session_id;
	/**
 	* @stopped_reason: reason why the session is being stopped:
	* 0x01: UCI_SESSION_REASON_MAX_NUMBER_OF_MEASUREMENTS_REACHED.
	* 0x02: UCI_SESSION_REASON_MAX_RANGING_ROUND_RETRY_COUNT_REACHED.
	* negative value: session is not being stopped.
	*/
	int16_t stopped_reason;
};

/**
 * struct device_params - Definition of device specific parameters
 */
struct device_params {
	/**
 	* @device_state: current uci device state.
	*/
	enum uci_device_state device_state;
	/**
 	* @low_power_mode: current low power mode:
	* 0x00: disabled.
	* 0x01: enabled.
	*/
	uint8_t low_power_mode;
	/**
 	* @default_channel: the device default channel.
	*/
	uint8_t default_channel;
};

/**
 * struct uci_backend_fira_context - Definition of this backend context.
 */
struct uci_backend_fira_context {
	/**
	 * @fira_context: context for fira helper.
	 */
	struct fira_context fira_context;
	/**
	 * @uwbmac_context: UWB MAC context.
	 */
	struct uwbmac_context *uwbmac_context;
	/**
	 * @antennas: antennas parameters.
	 */
	struct antenna_parameters *antennas;
	/**
	 * @core_device_reset_cb: callback for core device reset.
	 */
	void (*core_device_reset_cb)(uint8_t reason, void *user_data);
	/**
	 * @core_device_reset_cb_user_data: parameters for core_device_reset_cb.
	 */
	void *core_device_reset_cb_user_data;
	/**
	 * @running_sessions: number of running sessions.
	 */
	size_t running_sessions;
	/**
	 * @stop_ntf_policy: stop notification policy.
	 */
	struct stop_ntf_policy stop_ntf_policy;
	/**
	 * @device_param: devices parameters.
	 */
	struct device_params device_param;
	/**
	 * @uci: uci context used in callbacks from the helper.
	 */
	struct uci *uci;
	/**
	 * @coord: uci backend coordinator.
	 */
	struct uci_backend_coordinator *coord;
	/**
	 * @sess_man: the root backend manager.
	 */
	struct uci_backend_manager *sess_man;
};

/**
 * uci_backend_fira_init() - Attach this backend to the uci context to bridge uci communication to the MAC.
 * @context: FIRA context.
 * @uci: UCI context.
 * @uwbmac_context: UWB MAC context.
 * @coord: backend coordinator.
 * @sess_man: session manager.
 *
 * Return: UWBMAC_SUCCESS or an error code.
 */
uwbmac_error uci_backend_fira_init(struct uci_backend_fira_context *context,
				   struct uci *uci,
				   struct uwbmac_context *uwbmac_context,
				   struct uci_backend_coordinator *coord,
				   struct uci_backend_manager *sess_man);

/**
 * uci_backend_fira_set_reset_callback() - Set for core device reset callback.
 * @context: This backend context.
 * @cb: The callback to call on reset, or NULL.
 * @user_data: The callback private data.
 */
void uci_backend_fira_set_reset_callback(
	struct uci_backend_fira_context *context,
	void (*cb)(uint8_t reason, void *user_data), void *user_data);

/**
 * uci_backend_fira_send_reset_response() - Send response to core device reset.
 * @uci: UCI context.
 * @success: Status of the reset command.
 *
 * Must be called once done resetting the chip, to send the UCI response back.
 */
void uci_backend_fira_send_reset_response(struct uci *uci, bool success);

/**
 * uci_backend_fira_set_antenna_conf() - Give backend the antenna conf to configure session according to aoa requests.
 * @context: This backend context.
 * @antennas_params: The antenna configuration.
 */
void uci_backend_fira_set_antenna_conf(
	struct uci_backend_fira_context *context,
	struct antenna_parameters *antennas_params);

/**
 * uci_backend_fira_release() - Free global ressources used.
 * @context: This backend context.
 */
void uci_backend_fira_release(struct uci_backend_fira_context *context);

#endif // UCI_BACKEND_FIRA_H
