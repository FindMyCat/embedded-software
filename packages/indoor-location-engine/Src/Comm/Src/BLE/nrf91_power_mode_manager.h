#ifndef NRF91_POWER_MODE_MANAGER_H
#define NRF91_POWER_MODE_MANAGER_H

/* This section of code manages the power mode of nrf91 connected to the GPIO pins 
* When the BLE scanner on the Tag (this device) has seen the homestation recently (within 60 seconds)
* the device will command nrf91 to stay in PSM (Power saving mode).

* When the BLE scanner hasn't seen the homestation recently (within 60 seconds) then the device will command
* nrf91 to switch to EDRX mode (Enhanced Discontinuous Reception) so that it can receive messages from the Cloud.

* Pins used:
* P0.06 - nrf52833 (M_PIN24)
* TBD   - nrf91  

* Convention:
* 1 - High - POWER_COMMAND_PSM
* 0 - Low  - POWER_COMMAND_EDRX

We use the high level to stay in PSM mode, and when low, switch to EDRX mode. This is so that in case of a short circuit or some other mishap
the device will default to EDRX mode and not PSM mode. 

*/


void home_station_seen();
void power_mode_manager_thread_init();

#endif // NRF91_POWER_MODE_MANAGER_H