#include "wifi_geolocation.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zephyr/data/json.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/http/client.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/tls_credentials.h>

LOG_MODULE_REGISTER(wifi_geolocation, CONFIG_WIFI_LOG_LEVEL);

#define CA_CERT_TAG 1

#define REQUEST_BUF_SIZE 1024
#define RECV_BUF_SIZE 1024
#define BODY_BUF_SIZE 512

static const unsigned char ca_certificate[] = {
#include "gts_root_r1.pem.inc"
    0x00
};

struct response_context {
    char body[BODY_BUF_SIZE];
    size_t body_len;
    uint16_t status;
    bool complete;
};

struct geolocation_position {
    struct json_obj_token lat;
    struct json_obj_token lng;
};

struct geolocation_response {
    struct geolocation_position location;
    struct json_obj_token accuracy;
};

static const struct json_obj_descr position_descr[] = {
    JSON_OBJ_DESCR_PRIM(struct geolocation_position, lat, JSON_TOK_FLOAT),
    JSON_OBJ_DESCR_PRIM(struct geolocation_position, lng, JSON_TOK_FLOAT),
};

static const struct json_obj_descr response_descr[] = {
    JSON_OBJ_DESCR_OBJECT(struct geolocation_response, location,
                          position_descr),
    JSON_OBJ_DESCR_PRIM(struct geolocation_response, accuracy, JSON_TOK_FLOAT),
};

static int build_request_body(const struct wifi_scan_report *report, char *buf,
                              size_t buf_len) {
  int written = snprintf(buf, buf_len,
                         "{\"considerIp\":false,\"wifiAccessPoints\":[");

  if (written < 0 || (size_t)written >= buf_len) {
    return -ENOMEM;
  }

  for (size_t i = 0; i < report->count; i++) {
    const struct wifi_access_point *access_point = &report->access_points[i];
    int n = snprintf(buf + written, buf_len - written,
                     "%s{\"macAddress\":\"%02x:%02x:%02x:%02x:%02x:%02x\","
                     "\"signalStrength\":%d}",
                     i ? "," : "", access_point->bssid[0],
                     access_point->bssid[1], access_point->bssid[2],
                     access_point->bssid[3], access_point->bssid[4],
                     access_point->bssid[5], access_point->rssi);

    if (n < 0 || (size_t)n >= buf_len - written) {
      return -ENOMEM;
    }
    written += n;
  }

  int n = snprintf(buf + written, buf_len - written, "]}");

  if (n < 0 || (size_t)n >= buf_len - written) {
    return -ENOMEM;
  }

  return written + n;
}

/* Returns 0 to keep the download going, negative to abort it. */
static int response_handler(struct http_response *rsp,
                            enum http_final_call final_data, void *user_data) {
  struct response_context *ctx = user_data;

  ctx->status = rsp->http_status_code;

  if (rsp->body_frag_start && rsp->body_frag_len) {
    size_t room = sizeof(ctx->body) - 1 - ctx->body_len;
    size_t copy = MIN(room, rsp->body_frag_len);

    memcpy(ctx->body + ctx->body_len, rsp->body_frag_start, copy);
    ctx->body_len += copy;
    ctx->body[ctx->body_len] = '\0';
  }

  if (final_data == HTTP_DATA_FINAL) {
    ctx->complete = true;
  }

  return 0;
}

static int register_ca_certificate(void) {
  static bool registered;

  if (registered) {
    return 0;
  }

  int err = tls_credential_add(CA_CERT_TAG, TLS_CREDENTIAL_CA_CERTIFICATE,
                               ca_certificate, sizeof(ca_certificate));

  if (err && err != -EEXIST) {
    LOG_ERR("Error: registering CA certificate failed (%d)", err);
    return err;
  }

  registered = true;
  return 0;
}

static int connect_tls(void) {
  struct zsock_addrinfo hints = {
      .ai_family = AF_INET,
      .ai_socktype = SOCK_STREAM,
  };
  struct zsock_addrinfo *addr;

  int err = zsock_getaddrinfo(CONFIG_WIFI_GEOLOCATION_HOST, "443", &hints,
                              &addr);

  if (err) {
    LOG_ERR("Error: resolving %s failed (%d)", CONFIG_WIFI_GEOLOCATION_HOST,
            err);
    return -EIO;
  }

  int sock = zsock_socket(addr->ai_family, addr->ai_socktype, IPPROTO_TLS_1_2);

  if (sock < 0) {
    LOG_ERR("Error: creating TLS socket failed (%d)", -errno);
    zsock_freeaddrinfo(addr);
    return -errno;
  }

  sec_tag_t sec_tag_list[] = {CA_CERT_TAG};
  int verify = TLS_PEER_VERIFY_REQUIRED;

  if (zsock_setsockopt(sock, SOL_TLS, TLS_SEC_TAG_LIST, sec_tag_list,
                       sizeof(sec_tag_list)) < 0 ||
      zsock_setsockopt(sock, SOL_TLS, TLS_PEER_VERIFY, &verify,
                       sizeof(verify)) < 0 ||
      zsock_setsockopt(sock, SOL_TLS, TLS_HOSTNAME,
                       CONFIG_WIFI_GEOLOCATION_HOST,
                       sizeof(CONFIG_WIFI_GEOLOCATION_HOST)) < 0) {
    LOG_ERR("Error: configuring TLS socket failed (%d)", -errno);
    zsock_close(sock);
    zsock_freeaddrinfo(addr);
    return -EIO;
  }

  if (zsock_connect(sock, addr->ai_addr, addr->ai_addrlen) < 0) {
    LOG_ERR("Error: connecting to %s failed (%d)",
            CONFIG_WIFI_GEOLOCATION_HOST, -errno);
    zsock_close(sock);
    zsock_freeaddrinfo(addr);
    return -EIO;
  }

  zsock_freeaddrinfo(addr);
  return sock;
}

static int parse_response(char *body, size_t body_len,
                          struct wifi_location *out) {
  struct geolocation_response parsed = {0};

  int ret = json_obj_parse(body, body_len, response_descr,
                           ARRAY_SIZE(response_descr), &parsed);

  /* json_obj_parse returns a bitmask of the fields it decoded. */
  if (ret < 0 || !(ret & BIT(0))) {
    LOG_ERR("Error: parsing geolocation response failed (%d)", ret);
    return -EBADMSG;
  }

  out->latitude = strtod(parsed.location.lat.start, NULL);
  out->longitude = strtod(parsed.location.lng.start, NULL);
  out->accuracy_m =
      (ret & BIT(1)) ? strtod(parsed.accuracy.start, NULL) : 0.0;

  return 0;
}

int wifi_geolocation_resolve(const struct wifi_scan_report *report,
                             struct wifi_location *out) {
  static char request_body[REQUEST_BUF_SIZE];
  static uint8_t recv_buf[RECV_BUF_SIZE];
  struct response_context ctx = {0};
  char url[128];

  if (!report || !out) {
    return -EINVAL;
  }

  if (strlen(CONFIG_WIFI_GEOLOCATION_API_KEY) == 0) {
    LOG_ERR("Error: CONFIG_WIFI_GEOLOCATION_API_KEY is empty, set it in "
            "local.conf");
    return -EINVAL;
  }

  /* The API rejects a single access point. */
  if (report->count < 2) {
    LOG_WRN("Only %u access points, need at least 2", (unsigned)report->count);
    return -ENODATA;
  }

  int body_len = build_request_body(report, request_body, sizeof(request_body));

  if (body_len < 0) {
    LOG_ERR("Error: building request body failed (%d)", body_len);
    return body_len;
  }

  int err = register_ca_certificate();

  if (err) {
    return err;
  }

  int sock = connect_tls();

  if (sock < 0) {
    return sock;
  }

  snprintf(url, sizeof(url), "/geolocation/v1/geolocate?key=%s",
           CONFIG_WIFI_GEOLOCATION_API_KEY);

  /* Without Connection: close the server keeps the socket open after the
   * response, and the client waits out its whole timeout. */
  static const char *headers[] = {"Content-Type: application/json\r\n",
                                  "Connection: close\r\n", NULL};

  struct http_request req = {
      .method = HTTP_POST,
      .url = url,
      .host = CONFIG_WIFI_GEOLOCATION_HOST,
      .protocol = "HTTP/1.1",
      .header_fields = headers,
      .payload = request_body,
      .payload_len = (size_t)body_len,
      .response = response_handler,
      .recv_buf = recv_buf,
      .recv_buf_len = sizeof(recv_buf),
  };

  err = http_client_req(sock, &req,
                        CONFIG_WIFI_GEOLOCATION_TIMEOUT_S * MSEC_PER_SEC, &ctx);

  zsock_close(sock);

  if (err < 0) {
    LOG_ERR("Error: geolocation request failed (%d), status %u, %u body bytes, "
            "complete %d",
            err, ctx.status, (unsigned)ctx.body_len, ctx.complete);
    if (ctx.body_len) {
      LOG_ERR("Partial body: %s", ctx.body);
    }
    return err;
  }

  if (ctx.status != 200) {
    LOG_ERR("Error: geolocation API returned %u: %s", ctx.status, ctx.body);
    return -EIO;
  }

  return parse_response(ctx.body, ctx.body_len, out);
}
