#pragma once

typedef enum {
   SEMTRACKER_CMD_BUTTON,
   SEMTRACKER_CMD_WAKEUP,

} semtracker_cmd_e;

void semtracker_init (void);
void semtracker_cmd  (semtracker_cmd_e, uint32_t arg0);
