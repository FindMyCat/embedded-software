#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

/**
 * Ask each subsystem to log what it can prove about its hardware, then return
 * so the application continues normally. Each check lives in the module that
 * owns the peripheral; this only decides what runs and in what order.
 */
void diagnostics_run(void);

/**
 * Bring every radio down and return to measure the floor current of the FindMyCat PCB
 */
void diagnostics_low_power_hold(void);

#endif /* DIAGNOSTICS_H */
