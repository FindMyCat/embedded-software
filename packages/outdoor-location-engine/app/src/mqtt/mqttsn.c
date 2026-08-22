
#include <zephyr/kernel.h>
#include <stdio.h>
#include <errno.h>

#include <zephyr/net/mqtt_sn.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>
#include "../utils/network.h"
#include "mqttsn.h"

#define APP_BMEM
#define APP_DMEM

static APP_BMEM struct mqtt_sn_client client;
static APP_BMEM struct mqtt_sn_transport_udp tp;
static APP_BMEM struct mqtt_sn_data client_id = MQTT_SN_DATA_STRING_LITERAL(CONFIG_DEVICE_ID);

static uint8_t tx_buf[CONFIG_MQTT_SN_BUFFER_SIZE];
static uint8_t rx_buf[CONFIG_MQTT_SN_BUFFER_SIZE];

static bool mqtt_sn_connected = false;

LOG_MODULE_REGISTER(mqttsn, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

/**
 * @brief Callback function for MQTT-SN events.
 */
void evt_cb(struct mqtt_sn_client *client, const struct mqtt_sn_evt *evt)
{
	switch (evt->type) {
	case MQTT_SN_EVT_CONNECTED: /* Connected to a gateway */
		LOG_INF("MQTT-SN event EVT_CONNECTED");
		mqtt_sn_connected = true;
		break;
	case MQTT_SN_EVT_DISCONNECTED: /* Disconnected */
		LOG_INF("MQTT-SN event EVT_DISCONNECTED");
		mqtt_sn_connected = false;
		break;
	case MQTT_SN_EVT_ASLEEP: /* Entered ASLEEP state */
		LOG_INF("MQTT-SN event EVT_ASLEEP");
		break;
	case MQTT_SN_EVT_AWAKE: /* Entered AWAKE state */
		LOG_INF("MQTT-SN event EVT_AWAKE");
		break;
	case MQTT_SN_EVT_PUBLISH: /* Received a PUBLISH message */
		LOG_INF("MQTT-SN event EVT_PUBLISH");
		LOG_HEXDUMP_INF(evt->param.publish.data.data, evt->param.publish.data.size,
				"Published data");
		break;
	case MQTT_SN_EVT_PINGRESP: /* Received a PINGRESP */
		LOG_INF("MQTT-SN event EVT_PINGRESP");
		break;
	}
}

/**
 * @brief Function to initialize MQTT-SN connection.
 */
int mqttsn_initialize() {
	static bool client_initialized;
	int err;
	struct sockaddr_in gateway = {0};

	/* mqtt_sn_client_init() memsets the client, orphaning any gateway it
	 * still holds. Release it first or the slab runs out after a few
	 * reconnects, and add_gw then fails with "no free slot". */
	if (client_initialized) {
		mqtt_sn_client_deinit(&client);
	}
	client_initialized = true;

	char *ip_str = getIpAddressFromHostname(CONFIG_FINDMYCAT_CLOUD_HOSTNAME);
	gateway.sin_family = AF_INET;
	gateway.sin_port = htons(CONFIG_MQTT_SN_GATEWAY_PORT);
	err = zsock_inet_pton(AF_INET, ip_str, &gateway.sin_addr);

	// // __ASSERT(err == 0, "zsock_inet_pton() failed %d", err);

	LOG_INF("Connecting to MQTT-SN gateway %s:%d as client %s, topic %s", ip_str,
		CONFIG_MQTT_SN_GATEWAY_PORT, CONFIG_DEVICE_ID,
		CONFIG_MQTT_SN_PUBLISH_TOPIC);

	err = mqtt_sn_transport_udp_init(&tp, (struct sockaddr *)&gateway, sizeof((gateway)));
	if (err) {
		LOG_ERR("Error: mqtt_sn_transport_udp_init() failed: %d", err);
		return err;
	}

	err = mqtt_sn_client_init(&client, &client_id, &tp.tp, evt_cb, tx_buf, sizeof(tx_buf),
				  rx_buf, sizeof(rx_buf));
	/* The UDP transport sets SO_REUSEADDR, which the offloaded socket stack on
	 * this modem does not implement. The socket itself is already open and
	 * usable, so carry on rather than failing the whole connection. */
	if (err == ENOPROTOOPT || err == -ENOPROTOOPT) {
		LOG_WRN("SO_REUSEADDR unsupported, continuing");
	} else if (err) {
		LOG_ERR("Error: mqtt_sn_client_init() failed: %d", err);
		return err;
	}

	/* Zephyr 4.x sends to a registered gateway rather than to the transport
	 * address, so a static gateway must be added explicitly. */
	struct mqtt_sn_data gw_addr = {
		.data = (const uint8_t *)&gateway,
		.size = sizeof(gateway),
	};

	err = mqtt_sn_add_gw(&client, CONFIG_MQTT_SN_GATEWAY_ID, gw_addr);
	if (err) {
		LOG_ERR("Error: mqtt_sn_add_gw() failed: %d", err);
		return err;
	}

	err = mqtt_sn_connect(&client, false, true);
	if (err) {
		LOG_ERR("Error: mqtt_sn_connect() failed: %d", err);
	}

	return err;
}

/**
 * @brief Function to check for MQTT-SN input.
 */
int mqttsn_check_input() {
	int err = mqtt_sn_input(&client);
	if (err < 0) {
		LOG_ERR("failed: input: %d", err);
		return err;
	}

	return 0;
}


/**
 * @brief Function to publish a message to the MQTT-SN gateway.
 */
int mqttsn_publish(char *data_str) {
	if (!mqtt_sn_connected) {
		return -1;
	}
	static APP_DMEM struct mqtt_sn_data topic_p = MQTT_SN_DATA_STRING_LITERAL(CONFIG_MQTT_SN_PUBLISH_TOPIC);


	// Create a temporary mqtt_sn_data object to hold the message data
	struct mqtt_sn_data pubdata = {
		.data = (const uint8_t *)data_str, // cast data_str to const uint8_t pointer
		.size = strlen(data_str)
	};

	
	LOG_INF("Publishing to %s as client %s: %s", CONFIG_MQTT_SN_PUBLISH_TOPIC,
		CONFIG_DEVICE_ID, data_str);

	int err = mqtt_sn_publish(&client, MQTT_SN_QOS_0, &topic_p, false, &pubdata);
	if (err < 0) {
		LOG_ERR("Error: Publish to %s failed: %d", CONFIG_MQTT_SN_PUBLISH_TOPIC, err);
		return err;
	}

	/* Queued only: the topic still has to be registered with the gateway
	 * before the PUBLISH goes out, which needs mqtt_sn_input() pumping. */
	LOG_INF("Publish queued for %s", CONFIG_MQTT_SN_PUBLISH_TOPIC);

	return 0;
}

/**
 * @brief Function to disconnect from the MQTT-SN gateway.
 */
int mqttsn_disconnect() {
	/* Send the DISCONNECT before releasing the client: deinit destroys the
	 * gateway list, and without a gateway there is nowhere to send it. The
	 * gateway then keeps the session open and ignores our next CONNECT. */
	int err = mqtt_sn_disconnect(&client);
	if (err) {
		LOG_ERR("Error: mqtt_sn_disconnect() failed: %d", err);
	}

	mqtt_sn_client_deinit(&client);

	return err;
}

/**
 * @brief Function to get the MQTT-SN connection status.
 */
bool get_mqttsn_connection_status() {
	return mqtt_sn_connected;
}