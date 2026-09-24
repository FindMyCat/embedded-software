
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>

#include "../utils/network.h"
#include "mqttsn.h"
#include <zephyr/logging/log.h>
#include <zephyr/net/mqtt_sn.h>
#include <zephyr/net/socket.h>
#include "../shared.h"

#define APP_BMEM
#define APP_DMEM

#define MQTT_SN_CONNECT_TIMEOUT_MS 10000
#define MQTT_SN_CONNECT_POLL_MS    100
#define MQTT_SN_FLUSH_MS           2000
#define MQTT_SN_CONNECT_RETRIES    3

struct fmc_mqtt_sn_config {
    struct mqtt_sn_data client_id;
    char hostname[100];
    int port;
};

static struct fmc_mqtt_sn_config fmc_mqtt_config = {
	.client_id = MQTT_SN_DATA_STRING_LITERAL(UNCONFIGURED_DEVICE_ID),
	.hostname = CONFIG_FINDMYCAT_CLOUD_HOSTNAME,
	.port = CONFIG_MQTT_SN_GATEWAY_PORT};

static APP_BMEM struct mqtt_sn_client client;
static APP_BMEM struct mqtt_sn_transport_udp transport;
static APP_BMEM char client_id_buf[37];

static uint8_t tx_buf[CONFIG_MQTT_SN_BUFFER_SIZE];
static uint8_t rx_buf[CONFIG_MQTT_SN_BUFFER_SIZE];

static bool mqtt_sn_connected = false;
static bool client_initialized;

static K_MUTEX_DEFINE(session_lock);

LOG_MODULE_REGISTER(mqttsn, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

int initialize_mqttsn_config(const char *device_id, const char *hostname)
{
    if (device_id || device_id[0] ||
	strncmp(client_id_buf, device_id, sizeof(client_id_buf)) != 0) {

	strncpy(client_id_buf, device_id, sizeof(client_id_buf) - 1);
	fmc_mqtt_config.client_id.data = (const uint8_t *)client_id_buf;
	fmc_mqtt_config.client_id.size = strlen(client_id_buf);

	LOG_INF("MQTT-SN Client id is now %s", client_id_buf);
    }

    if (hostname || hostname[0] ||
	strncmp(fmc_mqtt_config.hostname, hostname, sizeof(fmc_mqtt_config.hostname)) != 0) {
	strncpy(fmc_mqtt_config.hostname, hostname, sizeof(fmc_mqtt_config.hostname) - 1);
    }

    // When connection parameters are changed, drop the existing connection as it is now stale.
    if (mqtt_sn_connected) {
	mqttsn_disconnect();
    }

    return 0;
}

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
int mqttsn_initialize()
{
    int err;
    struct sockaddr_in gateway = {0};

    /* mqtt_sn_client_init() memsets the client, orphaning any gateway it
     * still holds. Release it first or the slab runs out after a few
     * reconnects, and add_gw then fails with "no free slot". */
    if (client_initialized) {
	mqtt_sn_client_deinit(&client);
	client_initialized = false;
    }

    char *ip_str = getIpAddressFromHostname(fmc_mqtt_config.hostname);
    gateway.sin_family = AF_INET;
    gateway.sin_port = htons(fmc_mqtt_config.port);
    err = zsock_inet_pton(AF_INET, ip_str, &gateway.sin_addr);

    // // __ASSERT(err == 0, "zsock_inet_pton() failed %d", err);

    LOG_INF("Connecting to MQTT-SN gateway %s:%d as client %s, topic %s", ip_str,
	    CONFIG_MQTT_SN_GATEWAY_PORT, fmc_mqtt_config.client_id.data,
	    CONFIG_MQTT_SN_LOCATION_TOPIC);

    err = mqtt_sn_transport_udp_init(&transport, (struct sockaddr *)&gateway, sizeof((gateway)));
    if (err) {
	LOG_ERR("Error: mqtt_sn_transport_udp_init() failed: %d", err);
	return err;
    }

    err = mqtt_sn_client_init(&client, &fmc_mqtt_config.client_id, &transport.tp, evt_cb, tx_buf,
			      sizeof(tx_buf), rx_buf, sizeof(rx_buf));
    /* The UDP transport sets SO_REUSEADDR, which the offloaded socket stack on
     * this modem does not implement. The socket itself is already open and
     * usable, so carry on rather than failing the whole connection. */
    if (err == ENOPROTOOPT || err == -ENOPROTOOPT) {
	LOG_WRN("SO_REUSEADDR unsupported, continuing");
    } else if (err) {
	LOG_ERR("Error: mqtt_sn_client_init() failed: %d", err);
	return err;
    }

    client_initialized = true;

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
int mqttsn_check_input()
{
    if (!client_initialized) {
	return 0;
    }

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
int mqttsn_publish_topic(const char *topic, const char *data_str)
{
    if (!mqtt_sn_connected) {
	return -ENOTCONN;
    }

    /* mqtt_sn_publish() copies the name into its own topic entry, so a
     * caller owned string is enough here. */
    struct mqtt_sn_data topic_p = {.data = (const uint8_t *)topic, .size = strlen(topic)};

    // Create a temporary mqtt_sn_data object to hold the message data
    struct mqtt_sn_data pubdata = {
	    .data = (const uint8_t *)data_str, // cast data_str to const uint8_t pointer
	    .size = strlen(data_str)};

    LOG_INF("Publishing to %s as client %s: %s", topic, fmc_mqtt_config.client_id.data, data_str);

    int err = mqtt_sn_publish(&client, MQTT_SN_QOS_0, &topic_p, false, &pubdata);
    if (err < 0) {
	LOG_ERR("Error: Publish to %s failed: %d", topic, err);
	return err;
    }

    /* Queued only: the topic still has to be registered with the gateway
     * before the PUBLISH goes out, which needs mqtt_sn_input() pumping. */
    LOG_INF("Publish queued for %s", topic);

    return 0;
}

/**
 * @brief Publish one message, bringing up a gateway session if none is live.
 *
 * Connecting, pumping input until the CONNACK lands and flushing afterwards is
 * the same work whatever the payload is, so every caller shares it. The
 * publish is only queued when it returns: the topic still has to be registered
 * with the gateway, which the flush covers.
 *
 * @return 0 if the message was queued, negative errno otherwise.
 */
static int publish_with_session_locked(const char *topic, const char *data_str)
{
    int err = 0;

    for (int attempt = 0; attempt <= MQTT_SN_CONNECT_RETRIES; attempt++) {
	mqttsn_check_input();

	if (!mqtt_sn_connected) {
	    LOG_INF("MQTT-SN not connected. Initialising connection.");
	    err = mqttsn_initialize();

	    /* CONNACK only lands if input keeps being pumped, so poll for
	     * it rather than sleeping blind: a publish sent before the
	     * client is connected is dropped on the floor. */
	    for (int waited_ms = 0; waited_ms < MQTT_SN_CONNECT_TIMEOUT_MS && !mqtt_sn_connected;
		 waited_ms += MQTT_SN_CONNECT_POLL_MS) {
		mqttsn_check_input();
		k_sleep(K_MSEC(MQTT_SN_CONNECT_POLL_MS));
	    }
	}

	if (mqtt_sn_connected) {
	    break;
	}

	LOG_ERR("Error: MQTT-SN connection failed (%d), attempt %d", err, attempt + 1);
	mqttsn_disconnect();
    }

    if (!mqtt_sn_connected) {
	LOG_ERR("Error: MQTT-SN connection failed, giving up on %s", topic);
	return err ? err : -ENOTCONN;
    }

    mqttsn_check_input();

    err = mqttsn_publish_topic(topic, data_str);
    if (err) {
	return err;
    }

    /* A publish to a topic name is held until the gateway answers the
     * REGISTER with a REGACK, so keep pumping input afterwards. Returning
     * straight away lets the caller disconnect mid-registration and the
     * message never leaves the device. */
    for (int waited_ms = 0; waited_ms < MQTT_SN_FLUSH_MS; waited_ms += MQTT_SN_CONNECT_POLL_MS) {
	mqttsn_check_input();
	k_sleep(K_MSEC(MQTT_SN_CONNECT_POLL_MS));
    }

    mqttsn_disconnect();

    return 0;
}

#define SESSION_LOCK_TIMEOUT_MS                                                                    \
    (((MQTT_SN_CONNECT_RETRIES + 1) * MQTT_SN_CONNECT_TIMEOUT_MS) + MQTT_SN_FLUSH_MS +             \
     MQTT_SN_CONNECT_TIMEOUT_MS)

int mqttsn_publish_with_session(const char *topic, const char *data_str)
{
    if (k_mutex_lock(&session_lock, K_MSEC(SESSION_LOCK_TIMEOUT_MS)) != 0) {
	LOG_ERR("Error: another publish still holds the session, dropping %s", topic);
	return -EBUSY;
    }

    int err = publish_with_session_locked(topic, data_str);

    k_mutex_unlock(&session_lock);

    return err;
}

/**
 * @brief Function to disconnect from the MQTT-SN gateway.
 */
int mqttsn_disconnect()
{
    if (!client_initialized) {
	return 0;
    }

    /* Send the DISCONNECT before releasing the client: deinit destroys the
     * gateway list, and without a gateway there is nowhere to send it. The
     * gateway then keeps the session open and ignores our next CONNECT. */
    int err = mqtt_sn_disconnect(&client);
    if (err) {
	LOG_ERR("Error: mqtt_sn_disconnect() failed: %d", err);
    }

    mqtt_sn_client_deinit(&client);
    client_initialized = false;

    return err;
}

/**
 * @brief Function to get the MQTT-SN connection status.
 */
bool get_mqttsn_connection_status()
{
    return mqtt_sn_connected;
}
