#ifndef CA_CERTIFICATE_H
#define CA_CERTIFICATE_H

/* The certificate authority that signed the broker's certificate, in PEM.
 *
 *   openssl s_client -showcerts -connect <your-findmycat-cloud-domain>:8883 </dev/null
 */
static const unsigned char ca_certificate[] =
	"-----BEGIN CERTIFICATE-----\n"
	"MIIBpzCCAU6gAwIBAgIUT6hxIgXt+xK85zFnPAxiFEx8tQQwCgYIKoZIzj0EAwIw\n"
	"GzEZMBcGA1UEAwwQYXBpLmZpbmRteWNhdC5pbzAeFw0yNjA5MDEwNzEyMDFaFw0z\n"
	"NjA4MjkwNzEyMDFaMBsxGTAXBgNVBAMMEGFwaS5maW5kbXljYXQuaW8wWTATBgcq\n"
	"hkjOPQIBBggqhkjOPQMBBwNCAARpkl+7UzQvXyw2lmZnFjQUploea/Tl+6UUt8I9\n"
	"IC7KDZ0VgOnPXpXBNwtdOrWqqmZHYmMNWYXzJsmA1jlDYquHo3AwbjAdBgNVHQ4E\n"
	"FgQUAGNZOnsE05ZwRLT8Y0SrpR41o9MwHwYDVR0jBBgwFoAUAGNZOnsE05ZwRLT8\n"
	"Y0SrpR41o9MwDwYDVR0TAQH/BAUwAwEB/zAbBgNVHREEFDASghBhcGkuZmluZG15\n"
	"Y2F0LmlvMAoGCCqGSM49BAMCA0cAMEQCICNO8T+J/OB+uHm+f/XkkigZw/7hEZj7\n"
	"tFXT8U9BPqzAAiADUpfP3Lrx3xSJelTI7Ff5wDcnua0/84aRhB241SD/aQ==\n"
	"-----END CERTIFICATE-----\n";

#endif /* CA_CERTIFICATE_H */