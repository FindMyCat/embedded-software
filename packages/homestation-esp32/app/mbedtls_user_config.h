/*
 * Google's certificate chain arrives in a record far larger than the request
 * we send, so the buffers are sized independently. A symmetric
 * MBEDTLS_SSL_MAX_CONTENT_LEN big enough for the inbound record would waste
 * the same amount again on an outbound buffer that never holds more than a
 * few hundred bytes of JSON.
 */

#undef MBEDTLS_SSL_MAX_CONTENT_LEN

#define MBEDTLS_SSL_IN_CONTENT_LEN 16384
#define MBEDTLS_SSL_OUT_CONTENT_LEN 2048
