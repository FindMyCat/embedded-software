
#include <zephyr/kernel.h>
#include <stdio.h>
#include <errno.h>

#include <zephyr/net/mqtt_sn.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>

#define APP_BMEM
#define APP_DMEM

static APP_BMEM struct mqtt_sn_client client;
static APP_BMEM struct mqtt_sn_transport_udp tp;
static APP_BMEM struct mqtt_sn_data client_id = MQTT_SN_DATA_STRING_LITERAL("ZEPHYR");

static uint8_t tx_buf[CONFIG_NET_SAMPLE_MQTT_SN_BUFFER_SIZE];
static uint8_t rx_buf[CONFIG_NET_SAMPLE_MQTT_SN_BUFFER_SIZE];

static bool mqtt_sn_connected;

LOG_MODULE_REGISTER(mqttsn, 4);

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
	int err;
	struct sockaddr_in gateway = {0};

	LOG_DBG("Parsing MQTT host IP " CONFIG_NET_SAMPLE_MQTT_SN_GATEWAY_IP);
	gateway.sin_family = AF_INET;
	gateway.sin_port = htons(CONFIG_NET_SAMPLE_MQTT_SN_GATEWAY_PORT);
	err = zsock_inet_pton(AF_INET, CONFIG_NET_SAMPLE_MQTT_SN_GATEWAY_IP, &gateway.sin_addr);

	// // __ASSERT(err == 0, "zsock_inet_pton() failed %d", err);

	LOG_INF("Waiting for connection...");
	LOG_HEXDUMP_DBG(&gateway, sizeof(gateway), "gateway");

	LOG_INF("Connecting client");

	err = mqtt_sn_transport_udp_init(&tp, (struct sockaddr *)&gateway, sizeof((gateway)));
	__ASSERT(err == 0, "mqtt_sn_transport_udp_init() failed %d", err);

	err = mqtt_sn_client_init(&client, &client_id, &tp.tp, evt_cb, tx_buf, sizeof(tx_buf),
				  rx_buf, sizeof(rx_buf));
	__ASSERT(err == 0, "mqtt_sn_client_init() failed %d", err);

	err = mqtt_sn_connect(&client, false, true);
	__ASSERT(err == 0, "mqtt_sn_connect() failed %d", err);

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
int mqttsn_publish() {
	if (!mqtt_sn_connected) {
		return -1;
	}
	static APP_DMEM struct mqtt_sn_data topic_p = MQTT_SN_DATA_STRING_LITERAL("/location");

	char out[20];
	struct mqtt_sn_data pubdata = {.data = out};
	
	LOG_INF("Publishing timestamp");

	int err = snprintk(out, sizeof(out), "%" PRIi64, k_uptime_get());
	if (err < 0) {
		LOG_ERR("failed: snprintf");
		return err;
	}

	pubdata.size = MIN(sizeof(out), err);

	err = mqtt_sn_publish(&client, MQTT_SN_QOS_0, &topic_p, false, &pubdata);
	if (err < 0) {
		LOG_ERR("failed: publish: %d", err);
		return err;
	}

	return 0;
}