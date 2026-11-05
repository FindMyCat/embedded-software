#ifndef SUPL_ASSISTANCE_H
#define SUPL_ASSISTANCE_H

#include <nrf_modem_gnss.h>

int supl_assistance_fetch_and_inject(
    const struct nrf_modem_gnss_agnss_data_frame *requested_data);

#endif /* SUPL_ASSISTANCE_H */