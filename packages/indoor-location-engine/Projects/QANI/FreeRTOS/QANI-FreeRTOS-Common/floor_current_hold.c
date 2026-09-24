#include "floor_current_hold.h"

#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"

void floor_current_hold(void)
{
#if INDOOR_FLOOR_CURRENT_HOLD
    nrf_delay_ms(5000); /* SWD reattach window */

    /* DW3000 RSTn held low;*/
    nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0, 8));
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 8));
    nrf_gpio_cfg(NRF_GPIO_PIN_MAP(0, 17), NRF_GPIO_PIN_DIR_INPUT,
		 NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_PULLDOWN,
		 NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);
    nrf_gpio_cfg(NRF_GPIO_PIN_MAP(0, 22), NRF_GPIO_PIN_DIR_INPUT,
		 NRF_GPIO_PIN_INPUT_DISCONNECT, NRF_GPIO_PIN_PULLDOWN,
		 NRF_GPIO_PIN_S0S1, NRF_GPIO_PIN_NOSENSE);

    /* WAKEUP and CS wake the DW3000 if they float; no external pull. */
    nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0, 19));
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 19));
    nrf_gpio_pin_set(NRF_GPIO_PIN_MAP(0, 20));
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 20));

    nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0, 21)); /* SPI MOSI */
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 21));
    nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0, 23)); /* SPI SCK */
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 23));

    /* Q2 gate on the buzzer FET. A floating gate partially conducts off VSYS. */
    nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0, 27));
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 27));

    nrf_gpio_pin_clear(NRF_GPIO_PIN_MAP(0, 5)); /* config ready */
    nrf_gpio_cfg_output(NRF_GPIO_PIN_MAP(0, 5));

    NRF_POWER->SYSTEMOFF = 1;
    for (;;) {}
#endif
}