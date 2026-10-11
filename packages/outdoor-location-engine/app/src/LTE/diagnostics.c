#include "diagnostics.h"

#include <modem/lte_lc.h>
#include <modem/modem_info.h>
#include <nrf_modem_at.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "lte.h"

LOG_MODULE_REGISTER(lte_diag, CONFIG_FINDMYCAT_OUTDOOR_LOCATION_ENGINE_LOG_LEVEL);

/* Powers the UICC and so the SIM card can be read. */
#define CFUN_UICC_ONLY 41

/* %XMONITOR answers with the whole radio picture on one line. */
#define RESPONSE_BYTES 160

/* The modem's way of saying the holder is empty. */
#define CME_SIM_NOT_INSERTED 10

#define SIM_READY_TIMEOUT_MS 3000
#define SIM_POLL_INTERVAL_MS 100

static void report_sim(void)
{
    char response[RESPONSE_BYTES];
    int err = nrf_modem_at_printf("AT+CFUN=%d", CFUN_UICC_ONLY);

    if (err != 0) {
        LOG_ERR("  FAIL could not power the UICC (%d)", err);
        return;
    }

    /* Without this the modem answers a bare ERROR and the reason for a failure
     * is lost; with it, failures come back as +CME ERROR with a code. */
    (void)nrf_modem_at_printf("AT+CMEE=1");

    for (int waited = 0; waited < SIM_READY_TIMEOUT_MS;
         waited += SIM_POLL_INTERVAL_MS) {
        err = nrf_modem_at_cmd(response, sizeof(response), "AT+CPIN?");

        if (err == 0) {
            break;
        }

        /* An empty holder is not going to fill itself; only wait out the
         * errors that a card still starting up would give. */
        if (nrf_modem_at_err_type(err) == NRF_MODEM_AT_CME_ERROR &&
            nrf_modem_at_err(err) == CME_SIM_NOT_INSERTED) {
            LOG_ERR("  FAIL no SIM detected: check the card and the holder");
            return;
        }

        k_sleep(K_MSEC(SIM_POLL_INTERVAL_MS));
    }

    if (err != 0) {
        LOG_ERR("  FAIL SIM did not become readable within %d ms (%d)",
                SIM_READY_TIMEOUT_MS, err);
        return;
    }

    if (strstr(response, "READY") != NULL) {
        LOG_INF("  PASS SIM ready, no PIN required");
    } else {
        LOG_WRN("  SIM present but not ready: %s", response);
    }

    if (nrf_modem_at_cmd(response, sizeof(response), "AT%%XICCID") == 0) {
        LOG_INF("  %s", response);
    }
}

void lte_diagnostics_report_network(void)
{
    char response[RESPONSE_BYTES];

    if (nrf_modem_at_cmd(response, sizeof(response), "AT%%XMONITOR") == 0) {
        LOG_INF("  %s", response);
    }

    /* Energy estimate: 5 excessive, 6 increased, 7 normal, 8 reduced,
     * 9 efficient. A CE level above 0 means paging repeats, so each eDRX
     * window costs more. */
    struct lte_lc_conn_eval_params params = {0};
    int err = lte_lc_conn_eval_params_get(&params);

    if (err != 0) {
        LOG_WRN("  Connection evaluation failed (%d)", err);
        return;
    }

    LOG_INF("  RSRP %d dBm, CE level %d, energy estimate %d, pathloss %d dB",
            RSRP_IDX_TO_DBM(params.rsrp), params.ce_level, params.energy_estimate,
            params.dl_pathloss);
}

int lte_diagnostics(void)
{
    LOG_INF("SIM:");
    report_sim();

    LOG_INF("Network:");

    int err = lte_connect();

    if (err != 0) {
        LOG_ERR("  FAIL LTE attach (%d)", err);
        return err;
    }

    LOG_INF("  PASS attached");
    lte_diagnostics_report_network();

    return 0;
}