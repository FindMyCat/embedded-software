#ifndef CA_CERTIFICATE_H
#define CA_CERTIFICATE_H

/* The certificate authority that signed the broker's certificate, in PEM.
 *
 *   openssl s_client -showcerts -connect <your-findmycat-cloud-domain>:8883 </dev/null
 */
static const unsigned char ca_certificate[] = "";

#endif /* CA_CERTIFICATE_H */