#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/mqtt.h>
#include <zephyr/net/socket.h>

#include "ble_scanner.h"
#include "mqtt_publisher.h"
#include "wifi_station.h"
#if defined(CONFIG_MQTT_PUBLISHER_TLS)
#include <zephyr/net/tls_credentials.h>

#include "ca_certificate.h"

static const sec_tag_t sec_tag_list[] = {CONFIG_MQTT_PUBLISHER_SEC_TAG};
#endif

LOG_MODULE_REGISTER(mqtt_publisher, CONFIG_MQTT_PUBLISHER_LOG_LEVEL);

#define PAYLOAD_BYTES 128

#define PAYLOAD_FORMAT "{\"msg\": \"%s,%s,0,%d,%u\"}"

#define RX_BUFFER_BYTES 256
#define TX_BUFFER_BYTES 256

#define KEEPALIVE_S 60
#define CONNECT_RETRY_S 10

static uint8_t rx_buffer[RX_BUFFER_BYTES];
static uint8_t tx_buffer[TX_BUFFER_BYTES];

static struct mqtt_client client;
static struct sockaddr_storage broker;

static bool connected;

static void mqtt_event_handler(struct mqtt_client *c,
			       const struct mqtt_evt *evt)
{
	ARG_UNUSED(c);

	switch (evt->type) {
	case MQTT_EVT_CONNACK:
		if (evt->result != 0) {
			LOG_ERR("Error: broker refused the connection (%d)",
				evt->result);
			break;
		}

		connected = true;
		LOG_INF("Connected to the broker as %s",
			CONFIG_DEVICE_ID);
		break;
	case MQTT_EVT_DISCONNECT:
		connected = false;
		LOG_WRN("Disconnected from the broker (%d)", evt->result);
		break;
	case MQTT_EVT_PUBACK:
		LOG_DBG("Publish %u acknowledged", evt->param.puback.message_id);
		break;
	default:
		break;
	}
}

static void client_init(void)
{
	struct sockaddr_in *address = (struct sockaddr_in *)&broker;

	address->sin_family = AF_INET;
	address->sin_port = htons(CONFIG_MQTT_PUBLISHER_BROKER_PORT);

	mqtt_client_init(&client);

	client.broker = &broker;
	client.evt_cb = mqtt_event_handler;
	client.client_id.utf8 = (uint8_t *)CONFIG_DEVICE_ID;
	client.client_id.size = strlen(CONFIG_DEVICE_ID);
	client.password = NULL;
	client.user_name = NULL;
	client.protocol_version = MQTT_VERSION_3_1_1;
	client.keepalive = KEEPALIVE_S;
#if defined(CONFIG_MQTT_PUBLISHER_TLS)
	client.transport.type = MQTT_TRANSPORT_SECURE;

	struct mqtt_sec_config *tls = &client.transport.tls.config;

	tls->peer_verify = TLS_PEER_VERIFY_REQUIRED;
	tls->cipher_list = NULL;
	tls->sec_tag_list = sec_tag_list;
	tls->sec_tag_count = ARRAY_SIZE(sec_tag_list);
	tls->hostname = CONFIG_MQTT_PUBLISHER_BROKER_HOST;
#else
	client.transport.type = MQTT_TRANSPORT_NON_SECURE;
#endif
	client.rx_buf = rx_buffer;
	client.rx_buf_size = sizeof(rx_buffer);
	client.tx_buf = tx_buffer;
	client.tx_buf_size = sizeof(tx_buffer);
}

static int resolve_broker(void)
{
	struct sockaddr_in *address = (struct sockaddr_in *)&broker;

	/* A dotted quad needs no resolver, and this is the one form that works
	 * whether or not DNS came up. */
	if (zsock_inet_pton(AF_INET, CONFIG_MQTT_PUBLISHER_BROKER_HOST,
			    &address->sin_addr) == 1) {
		return 0;
	}

	struct zsock_addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_STREAM,
	};
	struct zsock_addrinfo *result;
	int err = zsock_getaddrinfo(CONFIG_MQTT_PUBLISHER_BROKER_HOST, NULL,
				    &hints, &result);

	if (err != 0) {
		LOG_ERR("Error: could not resolve %s (%d)",
			CONFIG_MQTT_PUBLISHER_BROKER_HOST, err);
		return -EIO;
	}

	address->sin_addr =
		((struct sockaddr_in *)result->ai_addr)->sin_addr;
	zsock_freeaddrinfo(result);

	char resolved[INET_ADDRSTRLEN];

	zsock_inet_ntop(AF_INET, &address->sin_addr, resolved,
			sizeof(resolved));
	LOG_INF("Broker %s resolved to %s", CONFIG_MQTT_PUBLISHER_BROKER_HOST,
		resolved);

	return 0;
}

static int process(int timeout_ms)
{
	struct zsock_pollfd fds = {
		.fd = client.transport.tcp.sock,
		.events = ZSOCK_POLLIN,
	};
	int64_t deadline = k_uptime_get() + timeout_ms;

	while (true) {
		int64_t remaining = deadline - k_uptime_get();

		if (remaining <= 0) {
			break;
		}

		if (client.transport.tcp.sock < 0) {
			break;
		}

		/* Never wait past the keepalive: mqtt_live has to run before it
		 * expires or the broker drops us. */
		int wait = MIN((int)remaining, mqtt_keepalive_time_left(&client));
		int ready = zsock_poll(&fds, 1, wait);

		if (ready < 0) {
			LOG_ERR("Error: poll failed (%d)", errno);
			return -errno;
		}

		if (ready > 0) {
			int err = mqtt_input(&client);

			if (err != 0) {
				LOG_ERR("Error: mqtt_input failed (%d)", err);
				return err;
			}
		}

		int err = mqtt_live(&client);

		if (err != 0 && err != -EAGAIN) {
			LOG_ERR("Error: mqtt_live failed (%d)", err);
			return err;
		}
	}

	return 0;
}

struct reported_position {
	double latitude;
	double longitude;
	double accuracy_m;
	uint8_t battery;
};

K_MSGQ_DEFINE(reported_positions, sizeof(struct reported_position), 2, 8);

void mqtt_publisher_report_position(double latitude, double longitude,
				    double accuracy_m, uint8_t battery)
{
	struct reported_position position = {
		.latitude = latitude,
		.longitude = longitude,
		.accuracy_m = accuracy_m,
		.battery = battery,
	};

	/* Full means the broker has been away; the newest fix is the useful one. */
	while (k_msgq_put(&reported_positions, &position, K_NO_WAIT) != 0) {
		struct reported_position stale;

		(void)k_msgq_get(&reported_positions, &stale, K_NO_WAIT);
	}
}

static int publish_location(const char *latitude, const char *longitude,
			    int accuracy_m, uint8_t battery)
{
	char payload[PAYLOAD_BYTES];

	int len = snprintf(payload, sizeof(payload), PAYLOAD_FORMAT, latitude,
			   longitude, accuracy_m, battery);

	if (len < 0 || len >= (int)sizeof(payload)) {
		LOG_ERR("Error: position payload did not fit");
		return -ENOMEM;
	}

	struct mqtt_publish_param param = {
		.message.topic.qos = MQTT_QOS_1_AT_LEAST_ONCE,
		.message.topic.topic.utf8 =
			(uint8_t *)CONFIG_MQTT_PUBLISHER_LOCATION_TOPIC,
		.message.topic.topic.size =
			strlen(CONFIG_MQTT_PUBLISHER_LOCATION_TOPIC),
		.message.payload.data = payload,
		.message.payload.len = (uint32_t)len,
		.message_id = (uint16_t)k_uptime_get_32(),
	};

	LOG_INF("Publishing %s to %s", payload,
		CONFIG_MQTT_PUBLISHER_LOCATION_TOPIC);

	return mqtt_publish(&client, &param);
}

static int publish_position(void)
{
	return publish_location(CONFIG_MQTT_PUBLISHER_HOME_LATITUDE,
				CONFIG_MQTT_PUBLISHER_HOME_LONGITUDE,
				CONFIG_MQTT_PUBLISHER_HOME_ACCURACY_M,
				ble_scanner_tag_battery_percent());
}

/* Returns non-zero on a publish failure, leaving the rest queued. */
static int publish_reported_positions(void)
{
	struct reported_position position;

	while (k_msgq_peek(&reported_positions, &position) == 0) {
		char latitude[16];
		char longitude[16];

		snprintf(latitude, sizeof(latitude), "%.6f", position.latitude);
		snprintf(longitude, sizeof(longitude), "%.6f",
			 position.longitude);

		int err = publish_location(latitude, longitude,
					   (int)position.accuracy_m,
					   position.battery);

		if (err != 0) {
			return err;
		}

		(void)k_msgq_get(&reported_positions, &position, K_NO_WAIT);
	}

	return 0;
}

#if defined(CONFIG_MQTT_PUBLISHER_TLS)
static int provision_ca_certificate(void)
{
	if (sizeof(ca_certificate) <= 1) {
		LOG_ERR("Error: no CA certificate built in, so the broker "
			"cannot be verified; fill in ca_certificate.h");
		return -ENOENT;
	}

	int err = tls_credential_add(CONFIG_MQTT_PUBLISHER_SEC_TAG,
				     TLS_CREDENTIAL_CA_CERTIFICATE,
				     ca_certificate, sizeof(ca_certificate));

	if (err == -EEXIST) {
		return 0;
	}

	if (err != 0) {
		LOG_ERR("Error: registering the CA certificate failed (%d)",
			err);
	}

	return err;
}
#endif

static void mqtt_task(void)
{
	client_init();

#if defined(CONFIG_MQTT_PUBLISHER_TLS)
	if (provision_ca_certificate() != 0) {
		LOG_ERR("Error: not publishing, TLS cannot be set up");
		return;
	}
#endif

	bool was_in_range = false;
	int64_t next_publish = 0;

	while (true) {
		if (!wifi_station_is_ready()) {
			k_sleep(K_SECONDS(CONNECT_RETRY_S));
			continue;
		}

		if (!connected) {
			if (resolve_broker() != 0) {
				k_sleep(K_SECONDS(CONNECT_RETRY_S));
				continue;
			}

			int err = mqtt_connect(&client);

			if (err != 0) {
				LOG_ERR("Error: mqtt_connect failed (%d), "
					"retrying in %d s", err, CONNECT_RETRY_S);
				k_sleep(K_SECONDS(CONNECT_RETRY_S));
				continue;
			}

			/* CONNACK arrives as an event, so give the broker a
			 * moment to answer before deciding it did not. */
			process(MSEC_PER_SEC * 5);

			if (!connected) {
				LOG_WRN("No CONNACK, retrying in %d s",
					CONNECT_RETRY_S);
				mqtt_abort(&client);
				k_sleep(K_SECONDS(CONNECT_RETRY_S));
				continue;
			}

			/* Publish on the next pass rather than waiting out an
			 * interval that started before the link existed. */
			next_publish = 0;
		}

		bool in_range = ble_scanner_tag_in_range();

		if (in_range && !was_in_range) {
			LOG_INF("Tag arrived, reporting it home");
			next_publish = 0;
		} else if (!in_range && was_in_range) {
			LOG_INF("Tag left, stopping reports");
		}

		was_in_range = in_range;

		if (publish_reported_positions() != 0) {
			LOG_ERR("Error: publishing a LoRa position failed, "
				"dropping the connection to start over");
			mqtt_abort(&client);
			connected = false;
			continue;
		}

		if (in_range && k_uptime_get() >= next_publish) {
			if (publish_position() != 0) {
				LOG_ERR("Error: publish failed, dropping the "
					"connection to start over");
				mqtt_abort(&client);
				connected = false;
				continue;
			}

			next_publish =
				k_uptime_get() +
				CONFIG_MQTT_PUBLISHER_INTERVAL_S * MSEC_PER_SEC;
		}

		/* Servicing the client is also how the loop paces itself. */
		if (process(MSEC_PER_SEC) != 0) {
			mqtt_abort(&client);
			connected = false;
		}
	}
}

/* Started from mqtt_publisher_start rather than at boot: the broker cannot be
 * reached before the WiFi station has an address. */
K_THREAD_DEFINE(mqtt_task_id, CONFIG_MQTT_PUBLISHER_THREAD_STACK_SIZE, mqtt_task,
		NULL, NULL, NULL, 5, 0, K_TICKS_FOREVER);

void mqtt_publisher_start(void) { k_thread_start(mqtt_task_id); }