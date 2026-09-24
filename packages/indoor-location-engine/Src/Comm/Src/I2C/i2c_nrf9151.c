#include "i2c_nrf9151.h"

#include <string.h>

#include "nrfx_twis.h"
#include "nrf_drv_gpiote.h"
#include "nrf_log.h"
#include "nrf_gpio.h"
#include "app_error.h"
#include "app_util_platform.h"

#include "buzzer.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#define I2C_NRF9151_SDA_PIN     NRF_GPIO_PIN_MAP(0, 26)
#define I2C_NRF9151_SCL_PIN     NRF_GPIO_PIN_MAP(0, 4)
#define CONFIG_RDY_PIN          NRF_GPIO_PIN_MAP(0, 5)

#define BUS_WINDOW_MAX_MS       1000

static const nrfx_twis_t m_twis = NRFX_TWIS_INSTANCE(1);

static TimerHandle_t m_window_timer;  /* releases our raise if the 91 never acks */
static volatile bool m_window_open;   /* we are driving the line */
static volatile bool m_twis_on;

static uint8_t m_buf[I2C_NRF9151_BUF_SIZE];
static uint8_t m_rx_scratch[I2C_NRF9151_BUF_SIZE + 1];
static uint8_t m_write_offset;
static volatile bool m_config_acked;
static volatile bool m_config_ack_pending;
static volatile uint8_t m_soc = I2C_NRF9151_SOC_UNKNOWN;

/* Matches fuel_gauge.c on the 91. */
struct __attribute__((packed)) batt_frame {
    float   voltage;
    float   current;
    float   temp;
    float   soc;
    float   tte;
    float   ttf;
    int32_t chg_status;
};

static void bus_window_close(void);

static void twis_set(bool on) {
    CRITICAL_REGION_ENTER();
    if (on != m_twis_on) {
        m_twis_on = on;
        if (on) {
            nrfx_twis_enable(&m_twis);
        } else {
            nrfx_twis_disable(&m_twis);
        }
    }
    CRITICAL_REGION_EXIT();
}

/* Either side raising or releasing the line lands here. */
static void line_evt_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action) {
    (void)pin;
    (void)action;
    uint32_t level = nrf_gpio_pin_read(CONFIG_RDY_PIN);
    NRF_LOG_INFO("DBG line edge: level=%u self=%u", level, m_window_open);
    twis_set(level != 0);
}

static void bus_window_open(void) {
    if (m_window_open) {
        xTimerReset(m_window_timer, 0);
        return;
    }
    m_window_open = true;
    twis_set(true);  /* before raising, so the 91 never sees us deaf */
    nrf_gpio_pin_set(CONFIG_RDY_PIN);
    /* No close timer means a window that never shuts. */
    if (xTimerStart(m_window_timer, 0) != pdPASS) {
        NRF_LOG_ERROR("Error: i2c window timer queue full");
        bus_window_close();
    }
}

/* Release our raise. TWIS stays on while the 91 still holds the line; the
 * falling edge turns it off. */
static void bus_window_close(void) {
    if (!m_window_open) {
        return;
    }
    m_window_open = false;
    nrf_gpio_pin_clear(CONFIG_RDY_PIN);
    NRF_LOG_INFO("DBG window released, line=%u", nrf_gpio_pin_read(CONFIG_RDY_PIN));
}

static void window_timer_cb(TimerHandle_t t) {
    (void)t;
    bus_window_close();
    if (m_config_acked) {
        m_config_acked = false;
        buzzer_play(BUZZER_TUNE_BEEP);
    }
}

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
                if (f.soc >= 0.f && f.soc <= 100.f) {
                    m_soc = (uint8_t)(f.soc + 0.5f); /* round, not truncate */
                }
            }
            if (m_write_offset == I2C_NRF9151_REG_CONFIG_APPLIED &&
                m_buf[I2C_NRF9151_REG_CONFIG_APPLIED] ==
                    m_buf[I2C_NRF9151_REG_CONFIG_VERSION]) {
                bus_window_close();
                if (m_config_ack_pending) {
                    m_config_ack_pending = false;
                    m_config_acked = true;
                    NRF_LOG_INFO("config v%u applied by 91, window closed",
                                 m_buf[I2C_NRF9151_REG_CONFIG_VERSION]);
                }
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
    /* D0S1: drive high, disconnect for low. Input stays connected for sense. */
    nrf_gpio_pin_clear(CONFIG_RDY_PIN);
    nrf_gpio_cfg(CONFIG_RDY_PIN, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_CONNECT,
                 NRF_GPIO_PIN_PULLDOWN, NRF_GPIO_PIN_D0S1, NRF_GPIO_PIN_NOSENSE);

    nrfx_twis_config_t cfg = {
        .addr               = { I2C_NRF9151_SLAVE_ADDR, 0 },
        .scl                = I2C_NRF9151_SCL_PIN,
        .sda                = I2C_NRF9151_SDA_PIN,
        .scl_pull           = NRF_GPIO_PIN_NOPULL,
        .sda_pull           = NRF_GPIO_PIN_NOPULL,
        .interrupt_priority = 2,
    };
    nrfx_err_t err = nrfx_twis_init(&m_twis, &cfg, twis_event_handler);
    if (err != NRFX_SUCCESS) {
        NRF_LOG_ERROR("Error: nrfx_twis_init 0x%x", err);
        return;
    }

    m_window_timer = xTimerCreate("i2cwin", pdMS_TO_TICKS(BUS_WINDOW_MAX_MS),
                                  pdFALSE, NULL, window_timer_cb);
    if (m_window_timer == NULL) {
        NRF_LOG_ERROR("Error: i2c_nrf9151 timer alloc failed");
        return;
    }

    if (!nrf_drv_gpiote_is_init()) {
        err = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err);
    }
    nrf_drv_gpiote_in_config_t line_cfg = GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
    line_cfg.skip_gpio_setup = true;
    err = nrf_drv_gpiote_in_init(CONFIG_RDY_PIN, &line_cfg, line_evt_handler);
    APP_ERROR_CHECK(err);
    nrf_drv_gpiote_in_event_enable(CONFIG_RDY_PIN, true);
    NRF_LOG_INFO("DBG line idle level=%u (expect 0)", nrf_gpio_pin_read(CONFIG_RDY_PIN));

    bus_window_open();
}

uint8_t i2c_nrf9151_soc(void) {
    return m_soc;
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

void i2c_nrf9151_publish_config(const struct fmc_config *cfg) {
    i2c_nrf9151_write(I2C_NRF9151_REG_CONFIG, (const uint8_t *)cfg, sizeof(*cfg));
    uint8_t ver = (uint8_t)(m_buf[I2C_NRF9151_REG_CONFIG_VERSION] + 1);
    if (ver == 0) {
        ver = 1;  /* 0 means "never published" to the 91 */
    }
    i2c_nrf9151_write(I2C_NRF9151_REG_CONFIG_VERSION, &ver, 1);
    m_config_ack_pending = true;
    bus_window_open();
    NRF_LOG_INFO("config published v%u, window open", ver);
}

void i2c_nrf9151_publish_test_config(void) {
    struct fmc_config cfg;
    memset(&cfg, 0, sizeof(cfg));
    strncpy(cfg.host, "test.findmycat.io", sizeof(cfg.host) - 1);
    cfg.port = 1885;
    strncpy(cfg.device_id, "dummy-device-0001", sizeof(cfg.device_id) - 1);
    i2c_nrf9151_publish_config(&cfg);
}
