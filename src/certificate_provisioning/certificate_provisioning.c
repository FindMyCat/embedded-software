#include "certificate_provisioning.h"
#include <stdio.h>
#include <zephyr/kernel.h>
#include <modem/modem_key_mgmt.h>

static const char PRIVATE_KEY[] = {
	#include "./certs/CA_certs/device_private_key.pem"
};

static const char DEVICE_CERTIFICATE[] = {
	#include "./certs/CA_certs/device_certificate.pem"
};

static const char AMAZON_ROOT_CA[] = {
	#include "./certs/CA_certs/Amazon_CA_root.pem"
};

/* Helpers for writing AT commands */
void resp_callback(const char *at_response)
{
        printf("AT response received:\n%s", at_response);
        printf("*************************\n");
}

void at_write(char* cmd)
{
        int err;

		printf("Command: %s\n" , cmd);
        err = nrf_modem_at_cmd_async(resp_callback, cmd, 1);
        if (err) {
                /* error */
				printf("Error: %d\n", err);
				return;
        }

        /* wait for response */
        k_sleep(K_SECONDS(10));
}

void delete_current_certificate_and_private_key(int sec_tag) {
    char cmd[50]; // Assuming a reasonable buffer size

    // Convert the integer to a string and concatenate it with the command
    snprintf(cmd, sizeof(cmd), "AT%%%%CMNG=3,%d,0", sec_tag);

    at_write(cmd);

    snprintf(cmd, sizeof(cmd), "AT%%%%CMNG=3,%d,1", sec_tag);

    at_write(cmd);

    snprintf(cmd, sizeof(cmd), "AT%%%%CMNG=3,%d,2", sec_tag);

    at_write(cmd);
}

void write_private_key() {
        int err = modem_key_mgmt_write(CONFIG_NRF_CLOUD_SEC_TAG,
                MODEM_KEY_MGMT_CRED_TYPE_PRIVATE_CERT,
                PRIVATE_KEY,
                strlen(PRIVATE_KEY));

    if (err) {
		printf("Failed to provision private key: %d", err);
		return err;
	} else {
        printf("Private key provisioned successfully\n");
    }
}

void write_device_certificate() {
    int err = modem_key_mgmt_write(CONFIG_NRF_CLOUD_SEC_TAG,
                MODEM_KEY_MGMT_CRED_TYPE_PUBLIC_CERT,
                DEVICE_CERTIFICATE,
                strlen(DEVICE_CERTIFICATE));

    if (err) {
		printf("Failed to provision device certificate: %d", err);
		return err;
	} else {
        printf("Device Certitificate provisioned successfully\n");
    }
}

void write_amazon_ca_certificate() {
    int err = modem_key_mgmt_write(CONFIG_NRF_CLOUD_SEC_TAG,
                MODEM_KEY_MGMT_CRED_TYPE_CA_CHAIN,
                AMAZON_ROOT_CA,
                strlen(AMAZON_ROOT_CA));

    if (err) {
		printf("Failed to provision Amazon CA certificate: %d", err);
		return err;
	} else {
        printf("Amazon CA Certitificate provisioned successfully\n");
    }
}

void certificate_provisioning_init(void)
{
    printf("certificate_provisioning_init\n");

    at_write("AT+CFUN=4");

    // delete_current_certificate_and_private_key(CONFIG_NRF_CLOUD_SEC_TAG);

    at_write("AT%%CMNG=1");

    // at_write("AT%%KEYGEN=16842753,2,0");

    // write_private_key();
    // write_device_certificate();
    // write_amazon_ca_certificate();

    at_write("AT%%CMNG=1");
}