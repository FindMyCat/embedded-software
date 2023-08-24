/*
 * Copyright (c) 2021-2022 Qorvo, Inc
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

#ifndef UCI_BACKEND_MANAGER_H
#define UCI_BACKEND_MANAGER_H

#include <uci/uci.h>
#include <uci/uci_message.h>
#include <uci/uci_spec_fira.h>
#include <uwbmac/uwbmac.h>

/**
 * DOC: Manager overview
 *
 * The uci_backend_manager implements command dispatching for the submodules it manages.
 * Various sub backends exists to handle FIRA, PCTT and Android CCC ranging commands in a modular way.
 * Thus, the manager will parse an UCI command and select the correct sub-backend to call based on session type.
 *
 */

static const int UCI_SESSION_TYPE_ALL = -1;

struct uci_session;

struct uci_session_controller;

/**
 * struct uci_backend_manager - Definition of this backend context.
 */
struct uci_backend_manager {
	/**
	 * @head: Linked list of sessions.
	 */
	struct uci_session *head;
	/**
	 * @uwbmac_context: UWB MAC context.
	 */
	struct uwbmac_context *uwbmac_context;
	/**
	 * @channel_number: Channel number in use.
	 */
	uint8_t channel_number;
	/**
	 * @uci: uci context used in callbacks from the helper.
	 */
	struct uci *uci;
	/**
	 * @core_device_info_vendor_data: vendor data to include in core device info.
	 */
	const uint8_t *core_device_info_vendor_data;
	/**
	 * @core_device_info_vendor_length: length of vendor_data.
	 */
	size_t core_device_info_vendor_length;
	/**
	 * @controller_head: Head of the list of uci session operations.
	 */
	struct uci_session_controller *controller_head;
	/**
	 * @uwb_disabled: Wether uwb is disabled or not
	 */
	bool uwb_disabled;
};

/**
 * uci_backend_manager_init() - Initialize the manager and attach this backend to the uci context.
 * @context: This backend context.
 * @uci: UCI context.
 * @uwbmac: UWB MAC context.
 */
void uci_backend_manager_init(struct uci_backend_manager *context,
			      struct uci *uci, struct uwbmac_context *uwbmac);

/**
 * uci_backend_manager_release() - Free ressources allocated.
 * @context: This backend context.
 */
void uci_backend_manager_release(struct uci_backend_manager *context);

/**
 * struct uci_core_controller_ops - The operations on core that are dispatched.
 */
struct uci_core_controller_ops {
	/**
	 * @get_device_info: Get device information.
	 */
	void (*get_device_info)(uint8_t *mac_major, uint8_t *mac_minor,
				uint8_t *phy_major, uint8_t *phy_minor,
				void *user_data);
};

/**
 * struct uci_session_controller_ops - The operations on session that are dispatched.
 */
struct uci_session_controller_ops {
	/**
	 * @init: Initialize a session.
	 * @uci: uci context needed to send uci messages.
	 * @mt_gid_oid: Union of message type, group identifier and opcode identifier.
	 * @session_id: The session identifier that shall be initialized.
	 * @user_data: user data previously passed in uci_backend_manager_register.
	 *
	 * Return: UWBMAC_SUCCESS or an error.
	 */
	uwbmac_error (*init)(struct uci *uci, uint16_t mt_gid_oid,
			     uint32_t session_id, void *user_data);
	/**
	 * @deinit: Deinitialize a session.
	 * @uci: uci context needed to send uci messages.
	 * @mt_gid_oid: Union of message type, group identifier and opcode identifier.
	 * @session_id: The session identifier that shall be deinitialized.
	 * @user_data: user data previously passed in uci_backend_manager_register.
	 *
	 * Return: UWBMAC_SUCCESS or an error.
	 */
	uwbmac_error (*deinit)(struct uci *uci, uint16_t mt_gid_oid,
			       uint32_t session_id, void *user_data);

	/**
	 * @start: Start the ranging of given session.
	 * @uci: uci context needed to send uci messages.
	 * @mt_gid_oid: Union of message type, group identifier and opcode identifier.
	 * @session_id: The identifier for the ranging that should start.
	 * @user_data: user data previously passed in uci_backend_manager_register.
	 *
	 * Return: UWBMAC_SUCCESS or an error.
	 */
	uwbmac_error (*start)(struct uci *uci, uint16_t mt_gid_oid,
			      uint32_t session_id, void *user_data);

	/**
	 * @stop: Stop the ranging of the given session.
	 * @uci: uci context needed to send uci messages.
	 * @mt_gid_oid: Union of message type, group identifier and opcode identifier.
	 * @session_id: The identifier for the ranging that should stop.
	 * @user_data: user data previously passed in uci_backend_manager_register.
	 *
	 * Return: UWBMAC_SUCCESS or an error.
	 */
	uwbmac_error (*stop)(struct uci *uci, uint16_t mt_gid_oid,
			     uint32_t session_id, void *user_data);

	/**
	 * @set_app_config: Configure a session.
	 * @uci: uci context needed to send uci messages.
	 * @mt_gid_oid: Union of message type, group identifier and opcode identifier.
	 * @payload: The whole uci message to process.
	 * @user_data: user data previously passed in uci_backend_manager_register.
	 *
	 * Return: UWBMAC_SUCCESS or an error.
	 */
	uwbmac_error (*set_app_config)(struct uci *uci, uint16_t mt_gid_oid,
				       struct uci_session *session,
				       const struct uci_blk *payload,
				       void *user_data);

	/**
	 * @get_app_config: Get session configuration.
	 * @uci: uci context needed to send uci messages.
	 * @mt_gid_oid: Union of message type, group identifier and opcode identifier.
	 * @payload: The whole uci message to process.
	 * @user_data: user data previously passed in uci_backend_manager_register.
	 *
	 * Return: UWBMAC_SUCCESS or an error.
	 */
	uwbmac_error (*get_app_config)(struct uci *uci, uint16_t mt_gid_oid,
				       const struct uci_blk *payload,
				       void *user_data);
};

/**
 * struct uci_session_controller - Definition of controller (or a sub-backend).
 */
struct uci_session_controller {
	/**
	 * @type: Type of session associated with this controller.
	 */
	enum uci_session_type type;
	/**
	 * @core_ops: Core ops of this controller.
	 */
	struct uci_core_controller_ops *core_ops;
	/**
	 * @session_ops: Session ops of this controller.
	 */
	struct uci_session_controller_ops *session_ops;
	/**
	 * @user_data: Data to pass to controller when calling an operation.
	 */
	void *user_data;
	/**
	 * @next: Pointer to next controller, or NULL if this
	 * is the last one. Filled at registration.
	 */
	struct uci_session_controller *next;
};

/**
 * uci_backend_manager_register() - Register a controller.
 * @manager: backend context
 * @core_ops: Controller core ops to register.
 * @session_ops: Controller session ops to register.
 * @type: Type of session to associate with ops.
 * @user_data: Data to give ops when calling it, used to pass along their context.
 */
void uci_backend_manager_register(struct uci_backend_manager *manager,
				  struct uci_core_controller_ops *core_ops,
				  struct uci_session_controller_ops *session_ops,
				  enum uci_session_type type, void *user_data);

/**
 * uci_backend_manager_unregister() - Remove all handlers associated to a session type.
 * @manager: backend context
 * @type: Session type.
 */
void uci_backend_manager_unregister(struct uci_backend_manager *manager,
				    enum uci_session_type type);

/**
 * uci_backend_manager_disable_uwb() - Disable uwb because it is used by another stack.
 * @manager: context.
 * @disable: true to disable, false to enable.
 *
 * On some hardware two stacks have access to the chipset. This is used to disable this stack when necessary.
 * It is enabled by default.
 */
void uci_backend_manager_disable_uwb(struct uci_backend_manager *manager,
				     bool disable);

/**
 * struct uci_session - Holds information on a session.
 */
struct uci_session {
	/**
	 * @next: Next element in list.
	 * NOTE: Private member
	 */
	struct uci_session *next; /* Private field, do not use */
	/**
	 * @destructor: Function to call to release all memory for this struct.
	 */
	void (*destructor)(struct uci_session *);
	/**
	 * @id: Unique ID.
	 * NOTE: Private member
	 */
	uint32_t id; /* Private field, do not overwrite */
	/**
	 * @type: Type as given at init.
	 */
	uint8_t type; /* Private field, do not overwrite */
	/**
	 * @channel_number: Channel number in use by the session, or 0 if none.
	 * NOTE: Private member
	 */
	uint8_t channel_number; /* Private field, do not overwrite */
	/**
	 * @is_active: Session is currently active, i.e. ranging.
	 * NOTE: Private member
	 */
	bool is_active; /* Private field, do not overwrite */
	/**
	 * @n_measurements: Cached current number of measurements.
	 */
	int n_measurements;
	/**
	 * @nb_range: Number of range measurements.
	 */
	uint8_t nb_range;
	/**
	 * @nb_elevation: Number of elevation measurements.
	 */
	uint8_t nb_elevation;
	/**
	 * @nb_azimuth: Number of azimuth measurements.
	 */
	uint8_t nb_azimuth;
	/**
	 * @aoa_result_req: The type of aoa report requested.
	 */
	uint8_t aoa_result_req;
};

/**
 * uci_session_destructor() - Simple destructor that just frees.
 * @session: Session to destroy.
 */
static inline void uci_session_destructor(struct uci_session *session)
{
	free(session);
}

/**
 * uci_session_add() - Add a new session.
 * @manager: Session manager to use.
 * @session_id: Session ID.
 * @session_type: Session type.
 * @destructor: Function to call to release all memory for this struct.
 * @session: Session to add.
 *
 * Return: UCI_STATUS_OK or
 *         UCI_STATUS_ERROR_SESSION_DUPLICATE (session with this ID exists).
 */
enum uci_status_code uci_session_add(struct uci_backend_manager *manager,
				     uint32_t session_id, uint8_t session_type,
				     void (*destructor)(struct uci_session *),
				     struct uci_session *session);

/**
 * uci_session_remove() - Remove and destroy a session.
 * @manager: Session manager to use.
 * @session_id: Session ID.
 * @session_type: Session type.
 *
 * Return: UCI_STATUS_OK or
 *         UCI_STATUS_ERROR_SESSION_NOT_EXIST (session does not exist).
 */
enum uci_status_code uci_session_remove(struct uci_backend_manager *manager,
					uint32_t session_id,
					uint8_t session_type);

/**
 * uci_session_get() - Get a session.
 * @manager: Session manager to use.
 * @session_id: Session ID.
 * @session_type: Session type.
 * @session: Pointer to session structure (can be NULL if not needed).
 *
 * Return: UCI_STATUS_OK or
 *         UCI_STATUS_ERROR_SESSION_NOT_EXIST (session does not exist) or
 *         UCI_STATUS_ERROR_SESSION_DUPLICATE (exists with a different type).
 */
enum uci_status_code uci_session_get(struct uci_backend_manager *manager,
				     uint32_t session_id, uint8_t session_type,
				     struct uci_session **session);

/**
 * uci_session_get_current_channel_number() - Get channel number in use.
 * @manager: Session manager to use.
 *
 * Return: channel number or 0 if none in use.
 */
uint8_t
uci_session_get_current_channel_number(struct uci_backend_manager *manager);

/**
 * uci_session_set_channel_number() - Set channel number for session.
 * @manager: Session manager to use.
 * @session_id: Session ID.
 * @channel_number: Channel number specified in app config.
 *
 * Return: UCI_STATUS_OK or
 *         UCI_STATUS_ERROR_SESSION_NOT_EXIST (session does not exist).
 */
enum uci_status_code
uci_session_set_channel_number(struct uci_backend_manager *manager,
			       uint32_t session_id, uint8_t channel_number);

/**
 * uci_set_channel() - set channel number in the device.
 * @manager: Session manager to use.
 * @session_id: Session ID.
 *
 * Return: UCI_STATUS_OK or
 *         UCI_STATUS_FAILED or
 *         UCI_STATUS_ERROR_ACTIVE_SESSIONS_ONGOING (another channel is already
 *         in use).
 */
enum uci_status_code uci_set_channel(struct uci_backend_manager *manager,
				     uint32_t session_id);

/**
 * uci_session_start() - Start ranging for session.
 * @manager: Session manager to use.
 * @session_id: Session ID.
 *
 * If no channel is un use:
 * - if session has specified a channel number, use it
 * - if session has not specified a channel number, use default
 *
 * return UCI_STATUS_OK
 *
 * If a channel number is in use (some sessions active), compare to session's:
 * - if session has none or same channel number, return UCI_STATUS_OK
 * - if session has same, return UCI_STATUS_ERROR_ACTIVE_SESSIONS_ONGOING
 *
 * Return: UCI_STATUS_OK or
 *         UCI_STATUS_ERROR_SESSION_NOT_EXIST (session does not exist) or
 *         UCI_STATUS_ERROR_SESSION_ACTIVE (session is already active) or
 *         UCI_STATUS_ERROR_ACTIVE_SESSIONS_ONGOING (another channel is already
 *         in use).
 */
enum uci_status_code uci_session_start(struct uci_backend_manager *manager,
				       uint32_t session_id);

/**
 * uci_session_stop() - Stop ranging for session.
 * @manager: Session manager to use.
 * @session_id: Session ID.
 *
 * Return: UCI_STATUS_OK or
 *         UCI_STATUS_ERROR_SESSION_NOT_EXIST (session does not exist) or
 *         UCI_STATUS_ERROR_SESSION_NOT_CONFIGURED (session is not active).
 */
enum uci_status_code uci_session_stop(struct uci_backend_manager *manager,
				      uint32_t session_id);

/**
 * uci_session_count() - Get sessions number.
 * @manager: Session manager to use.
 * @type: Type to count sessions for, or UCI_SESSION_TYPE_ALL for all types.
 *
 * Return: number of sessions in manager.
 */
int uci_session_count(struct uci_backend_manager *manager, int type);

/**
 * uci_session_clear() - Remove and destroy all sessions for a given type.
 * @manager: Session manager to use.
 * @type: Type to remove sessions for, or UCI_SESSION_TYPE_ALL for all types.
 * @cb: Function to run before destroying session.
 * @user_data: Cookie to pass to the callback above.
 */
void uci_session_clear(struct uci_backend_manager *manager, int type,
		       void (*cb)(struct uci_session *, void *),
		       void *user_data);

/**
 * uci_backend_manager_set_vendor_data() - Set for core device info vendor data.
 * @manager: Session manager to use.
 * @data: Pointer to vendor data to include in core device info response.
 * @length: The vendor data length.
 */
void uci_backend_manager_set_vendor_data(struct uci_backend_manager *manager,
					 const uint8_t *data, size_t length);

#endif // UCI_BACKEND_MANAGER_H
