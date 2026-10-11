#ifndef LTE_DIAGNOSTICS_H
#define LTE_DIAGNOSTICS_H

int lte_diagnostics(void);

/* Logs %XMONITOR and the modem's connection evaluation; needs LTE attached. */
void lte_diagnostics_report_network(void);

#endif /* LTE_DIAGNOSTICS_H */