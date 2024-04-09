#pragma once

#include <stdint.h>
#include <string.h>
#include "lorawan_key_config.h"
#include "smtc_modem_api.h"

#ifndef OLD_SWSD004
#include "smtc_modem_geolocation_api.h"
#endif

typedef struct {

   uint8_t
       dev_eui [ SMTC_MODEM_EUI_LENGTH ],
       join_eui[ SMTC_MODEM_EUI_LENGTH ];
   uint8_t
       app_key [ LORAWAN_APP_KEY_LEN ];
   bool
       airplane_mode;

#ifndef OLD_SWSD004
   smtc_modem_geolocation_send_mode_t
       geolocation_send_mode;
#endif
   smtc_modem_region_t
       region;

} config_t;

void     config_defaults (config_t *config);
uint32_t config_restore (config_t *config);
