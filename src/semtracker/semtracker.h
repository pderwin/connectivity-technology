#pragma once

typedef enum {
   SEMTRACKER_CMD_GNSS_SCAN,
   SEMTRACKER_CMD_WAKEUP,
   SEMTRACKER_CMD_WIFI_SCAN,
   SEMTRACKER_CMD_SIMULATE_DRIVEWAY_SENSOR,

} semtracker_cmd_e;

void semtracker_init (void);
void semtracker_cmd  (semtracker_cmd_e, uint32_t arg0);
