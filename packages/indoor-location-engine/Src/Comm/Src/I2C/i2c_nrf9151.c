#include "i2c_nrf9151.h"

#include <string.h>

#include "nrfx_twis.h"
#include "nrf_log.h"

#define I2C_NRF9151_SDA_PIN     NRF_GPIO_PIN_MAP(0, 26)
#define I2C_NRF9151_SCL_PIN     NRF_GPIO_PIN_MAP(0, 4)

static const nrfx_twis_t m_twis = NRFX_TWIS_INSTANCE(1);

static uint8_t m_buf[I2C_NRF9151_BUF_SIZE];
static uint8_t m_rx_scratch[I2C_NRF9151_BUF_SIZE + 1];
static uint8_t m_write_offset;

/* Battery frame the nRF9151 streams over this link (matches fuel_gauge.c). */
struct __attribute__((packed)) batt_frame {
    float   voltage;
    float   current;
    float   temp;
    float   soc;
    float   tte;
    float   ttf;
    int32_t chg_status;
};

static void twis_event_handler(nrfx_twis_evt_t const *p_event) {
    switch (p_event->type) {
        case NRFX_TWIS_EVT_WRITE_REQ:
            nrfx_twis_rx_prepare(&m_twis, m_rx_scratch, sizeof(m_rx_scratch));
            break;

        case NRFX_TWIS_EVT_WRITE_DONE: {
            size_t rx_len = p_event->data.rx_amount;
            NRF_LOG_INFO("i2c rx %u bytes:", (unsigned)rx_len);
            NRF_LOG_HEXDUMP_INFO(m_rx_scratch, rx_len);
            if (rx_len < 1) break;
            m_write_offset = m_rx_scratch[0];
            for (size_t i = 1; i < rx_len; i++) {
                uint8_t off = (uint8_t)(m_write_offset + (i - 1));
                m_buf[off] = m_rx_scratch[i];
            }
            if ((rx_len - 1) == sizeof(struct batt_frame)) {
                struct batt_frame f;
                memcpy(&f, &m_buf[m_write_offset], sizeof(f));
                NRF_LOG_INFO("batt: V=" NRF_LOG_FLOAT_MARKER " I=" NRF_LOG_FLOAT_MARKER,
                             NRF_LOG_FLOAT(f.voltage), NRF_LOG_FLOAT(f.current));
                NRF_LOG_INFO("      T=" NRF_LOG_FLOAT_MARKER " SoC=" NRF_LOG_FLOAT_MARKER,
                             NRF_LOG_FLOAT(f.temp), NRF_LOG_FLOAT(f.soc));
                NRF_LOG_INFO("      TTE=" NRF_LOG_FLOAT_MARKER " TTF=" NRF_LOG_FLOAT_MARKER,
                             NRF_LOG_FLOAT(f.tte), NRF_LOG_FLOAT(f.ttf));
                NRF_LOG_INFO("      chg=%d", (int)f.chg_status);
            }
            break;
        }

        case NRFX_TWIS_EVT_READ_REQ:
            nrfx_twis_tx_prepare(&m_twis, &m_buf[m_write_offset],
                                 I2C_NRF9151_BUF_SIZE - m_write_offset);
            break;

        case NRFX_TWIS_EVT_READ_DONE:
        case NRFX_TWIS_EVT_READ_ERROR:
        case NRFX_TWIS_EVT_WRITE_ERROR:
        case NRFX_TWIS_EVT_GENERAL_ERROR:
        default:
            break;
    }
}

void i2c_nrf9151_init(void) {
    nrfx_twis_config_t cfg = {
        .addr               = { I2C_NRF9151_SLAVE_ADDR, 0 },
        .scl                = I2C_NRF9151_SCL_PIN,
        .sda                = I2C_NRF9151_SDA_PIN,
        .scl_pull           = NRF_GPIO_PIN_PULLUP,
        .sda_pull           = NRF_GPIO_PIN_PULLUP,
        .interrupt_priority = 2,
    };
    nrfx_err_t err = nrfx_twis_init(&m_twis, &cfg, twis_event_handler);
    if (err != NRFX_SUCCESS) {
        NRF_LOG_ERROR("Error: nrfx_twis_init 0x%x", err);
        return;
    }
    nrfx_twis_enable(&m_twis);
    NRF_LOG_INFO("i2c_nrf9151 slave ready @0x%02x", I2C_NRF9151_SLAVE_ADDR);
}

int i2c_nrf9151_read(uint8_t offset, uint8_t *dst, size_t len) {
    if ((size_t)offset + len > I2C_NRF9151_BUF_SIZE) return -1;
    for (size_t i = 0; i < len; i++) dst[i] = m_buf[offset + i];
    return 0;
}

int i2c_nrf9151_write(uint8_t offset, const uint8_t *src, size_t len) {
    if ((size_t)offset + len > I2C_NRF9151_BUF_SIZE) return -1;
    for (size_t i = 0; i < len; i++) m_buf[offset + i] = src[i];
    return 0;
}
