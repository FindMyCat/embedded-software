#ifndef LORA_H
#define LORA_H

#include <stdbool.h>

void lora_start(void);

/* True while the collar has been heard within CONFIG_LORA_ABSENCE_S. */
bool lora_collar_in_range(void);

#endif /* LORA_H */