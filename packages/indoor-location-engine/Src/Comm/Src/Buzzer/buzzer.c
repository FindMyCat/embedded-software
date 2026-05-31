#include "buzzer.h"

#include "nrf_gpio.h"
#include "nrfx_pwm.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define BUZZER_PIN          NRF_GPIO_PIN_MAP(0, 27)
#define BUZZER_QUEUE_LEN    4
#define BUZZER_TASK_STACK   256
#define BUZZER_TASK_PRIO    1
#define PWM_BASE_CLOCK_HZ   1000000UL

typedef struct {
    uint16_t hz;
    uint16_t ms;
} note_t;

static const note_t TUNE_BOOT[] = {
    {2000, 150}, {3000, 150}
};

static const note_t TUNE_BEEP[] = {
    {2500, 80}
};

static const note_t TUNE_FIND[] = {
    {523, 120}, {659, 120}, {784, 120}, {1047, 240},
    {784,  80}, {1047, 200}
};

static const note_t TUNE_AIRTAG[] = {
    {1480, 80}, {0, 30},
    {1760, 80}, {0, 30},
    {2093, 80}, {0, 30},
    {2349, 120},
    {0, 250},
    {1480, 80}, {0, 30},
    {1760, 80}, {0, 30},
    {2093, 80}, {0, 30},
    {2349, 120},
    {0, 250},
    {1480, 80}, {0, 30},
    {1760, 80}, {0, 30},
    {2093, 80}, {0, 30},
    {2349, 180},
};

static const note_t TUNE_RICKROLL[] = {
    {1175, 180}, {1319, 180}, {880, 360}, {0, 60},
    {880, 180}, {1319, 180}, {1175, 540},
    {0, 200},

    {1175, 180}, {1319, 180}, {880, 360}, {0, 60},
    {880, 180}, {1175, 180}, {1319, 180}, {1175, 180}, {1047, 180}, {988, 360},
    {0, 200},

    {1047, 180}, {1175, 180}, {1047, 360}, {0, 60},
    {880, 180}, {988, 180}, {880, 540},
    {0, 200},

    {1175, 180}, {1319, 180}, {880, 360}, {0, 60},
    {880, 180}, {1319, 180}, {1175, 360}, {1047, 180}, {988, 180}, {880, 540},
};

static const struct {
    const note_t *notes;
    size_t count;
} TUNE_TABLE[BUZZER_TUNE_COUNT] = {
    [BUZZER_TUNE_BOOT]     = { TUNE_BOOT,     sizeof(TUNE_BOOT)     / sizeof(note_t) },
    [BUZZER_TUNE_BEEP]     = { TUNE_BEEP,     sizeof(TUNE_BEEP)     / sizeof(note_t) },
    [BUZZER_TUNE_FIND]     = { TUNE_FIND,     sizeof(TUNE_FIND)     / sizeof(note_t) },
    [BUZZER_TUNE_AIRTAG]   = { TUNE_AIRTAG,   sizeof(TUNE_AIRTAG)   / sizeof(note_t) },
    [BUZZER_TUNE_RICKROLL] = { TUNE_RICKROLL, sizeof(TUNE_RICKROLL) / sizeof(note_t) },
};

static QueueHandle_t m_queue;
static nrfx_pwm_t    m_pwm = NRFX_PWM_INSTANCE(0);
static nrf_pwm_values_common_t m_pwm_seq_value;
static nrf_pwm_sequence_t m_pwm_seq = {
    .values.p_common = &m_pwm_seq_value,
    .length          = 1,
    .repeats         = 0,
    .end_delay       = 0,
};

static void pwm_play_tone(uint16_t hz) {
    if (hz == 0) {
        nrfx_pwm_stop(&m_pwm, true);
        return;
    }
    uint16_t top = (uint16_t)(PWM_BASE_CLOCK_HZ / hz);
    m_pwm_seq_value = (top / 2) | 0x8000;
    nrfx_pwm_config_t cfg = {
        .output_pins  = { BUZZER_PIN, NRFX_PWM_PIN_NOT_USED, NRFX_PWM_PIN_NOT_USED, NRFX_PWM_PIN_NOT_USED },
        .irq_priority = 6,
        .base_clock   = NRF_PWM_CLK_1MHz,
        .count_mode   = NRF_PWM_MODE_UP,
        .top_value    = top,
        .load_mode    = NRF_PWM_LOAD_COMMON,
        .step_mode    = NRF_PWM_STEP_AUTO,
    };
    nrfx_pwm_init(&m_pwm, &cfg, NULL);
    nrfx_pwm_simple_playback(&m_pwm, &m_pwm_seq, 1, NRFX_PWM_FLAG_LOOP);
}

static void pwm_stop(void) {
    nrfx_pwm_stop(&m_pwm, true);
    nrfx_pwm_uninit(&m_pwm);
    nrf_gpio_cfg_output(BUZZER_PIN);
    nrf_gpio_pin_clear(BUZZER_PIN);
}

static void play_note(uint16_t hz, uint16_t ms) {
    if (ms == 0) return;
    if (hz == 0) {
        pwm_stop();
        vTaskDelay(pdMS_TO_TICKS(ms));
        return;
    }
    pwm_play_tone(hz);
    vTaskDelay(pdMS_TO_TICKS(ms));
}

static void buzzer_task(void *arg) {
    (void)arg;
    buzzer_tune_t tune;
    for (;;) {
        if (xQueueReceive(m_queue, &tune, portMAX_DELAY) == pdPASS) {
            if (tune >= BUZZER_TUNE_COUNT) continue;
            const note_t *notes = TUNE_TABLE[tune].notes;
            size_t count = TUNE_TABLE[tune].count;
            for (size_t i = 0; i < count; i++) {
                play_note(notes[i].hz, notes[i].ms);
            }
            pwm_stop();
        }
    }
}

void buzzer_init(void) {
    nrf_gpio_cfg_output(BUZZER_PIN);
    nrf_gpio_pin_clear(BUZZER_PIN);

    m_queue = xQueueCreate(BUZZER_QUEUE_LEN, sizeof(buzzer_tune_t));
    if (m_queue == NULL) return;

    xTaskCreate(buzzer_task, "buzzer", BUZZER_TASK_STACK,
                NULL, BUZZER_TASK_PRIO, NULL);
}

void buzzer_play(buzzer_tune_t tune) {
    if (m_queue == NULL) return;
    xQueueSend(m_queue, &tune, 0);
}
