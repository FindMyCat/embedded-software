#ifndef LORA_H
#define LORA_H

#include <stdbool.h>

void lora_start(void);

/* True while the collar has been heard within CONFIG_LORA_ABSENCE_S. */
bool lora_collar_in_range(void);

/* True while a locate is pinging and the collar has not acked yet. */
bool lora_locate_pinging(void);

/* True once the collar acked a locate ping, and for a minute after its
 * answer arrives. */
bool lora_locate_acked(void);

#endif /* LORA_H */