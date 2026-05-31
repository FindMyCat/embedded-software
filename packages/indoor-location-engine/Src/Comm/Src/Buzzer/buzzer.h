#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>

typedef enum {
    BUZZER_TUNE_BOOT,
    BUZZER_TUNE_BEEP,
    BUZZER_TUNE_FIND,
    BUZZER_TUNE_AIRTAG,
    BUZZER_TUNE_RICKROLL,
    BUZZER_TUNE_COUNT,
} buzzer_tune_t;

void buzzer_init(void);

void buzzer_play(buzzer_tune_t tune);

#endif
