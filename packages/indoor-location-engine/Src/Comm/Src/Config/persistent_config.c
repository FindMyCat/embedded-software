#include "persistent_config.h"

#include <string.h>

#include "fds.h"
#include "nrf_log.h"

#include "i2c_nrf9151.h"

/* in ble.c */
void app_ble_restore_persisted_config(const uint8_t *data, uint16_t len);

#define PERSISTENT_CONFIG_FILE  0xCF60
#define PERSISTENT_CONFIG_KEY   0x0001

static volatile bool      m_fds_ready;
static volatile bool      m_save_pending;
static struct fmc_config  m_pending_cfg;

/* FDS needs a word-aligned, word-multiple buffer */
static union {
    struct fmc_config cfg;
    uint32_t          words[(sizeof(struct fmc_config) + 3) / 4];
} m_rec_buf;

void persistent_config_save(const struct fmc_config *cfg) {
    if (!m_fds_ready) return;

    m_rec_buf.cfg = *cfg;

    fds_record_t rec = {
        .file_id           = PERSISTENT_CONFIG_FILE,
        .key               = PERSISTENT_CONFIG_KEY,
        .data.p_data       = m_rec_buf.words,
        .data.length_words = sizeof(m_rec_buf.words) / sizeof(uint32_t),
    };

    fds_record_desc_t desc = {0};
    fds_find_token_t  tok  = {0};
    ret_code_t rc;

    if (fds_record_find(PERSISTENT_CONFIG_FILE, PERSISTENT_CONFIG_KEY, &desc, &tok) == NRF_SUCCESS) {
        rc = fds_record_update(&desc, &rec);
    } else {
        rc = fds_record_write(&desc, &rec);
    }

    if (rc == FDS_ERR_NO_SPACE_IN_FLASH) {
        /* retry from the GC event */
        m_pending_cfg = *cfg;
        m_save_pending = true;
        fds_gc();
        return;
    }
    if (rc != NRF_SUCCESS) {
        NRF_LOG_ERROR("Error: fds config save 0x%x", rc);
    }
}

static void persistent_config_load(void) {
    fds_record_desc_t desc = {0};
    fds_find_token_t  tok  = {0};

    if (fds_record_find(PERSISTENT_CONFIG_FILE, PERSISTENT_CONFIG_KEY, &desc, &tok) != NRF_SUCCESS) {
        NRF_LOG_INFO("no saved config in flash");
        return;
    }

    fds_flash_record_t flash_rec = {0};
    if (fds_record_open(&desc, &flash_rec) != NRF_SUCCESS) {
        return;
    }
    struct fmc_config cfg;
    memcpy(&cfg, flash_rec.p_data, sizeof(cfg));
    fds_record_close(&desc);

    i2c_nrf9151_publish_config(&cfg);

    /* BLE read-back value: [port_lo][port_hi][host_len][host][device_id] */
    uint8_t wire[3 + sizeof(cfg.host) + sizeof(cfg.device_id)];
    size_t host_len = strnlen(cfg.host, sizeof(cfg.host));
    size_t id_len = strnlen(cfg.device_id, sizeof(cfg.device_id));
    wire[0] = (uint8_t)(cfg.port & 0xFF);
    wire[1] = (uint8_t)((cfg.port >> 8) & 0xFF);
    wire[2] = (uint8_t)host_len;
    memcpy(&wire[3], cfg.host, host_len);
    memcpy(&wire[3 + host_len], cfg.device_id, id_len);
    app_ble_restore_persisted_config(wire, (uint16_t)(3 + host_len + id_len));

    NRF_LOG_INFO("loaded config from flash port=%u", cfg.port);
}

static void fds_evt_handler(fds_evt_t const *p_evt) {
    switch (p_evt->id) {
        case FDS_EVT_INIT:
            if (p_evt->result == NRF_SUCCESS) {
                m_fds_ready = true;
                persistent_config_load();
            }
            break;
        case FDS_EVT_GC:
            if (m_save_pending) {
                m_save_pending = false;
                persistent_config_save(&m_pending_cfg);
            }
            break;
        default:
            break;
    }
}

void persistent_config_init(void) {
    (void)fds_register(fds_evt_handler);
}
