#pragma once

typedef enum {
   PE4259_SELECT_RF1 = 0x30,  // just non-zero for bug detection
   PE4259_SELECT_RF2,
} pe4259_select_e;

/*
 * Make board-specific aliases
 */
#define PE4259_SELECT_RF_WIFI  PE4259_SELECT_RF1
#define PE4259_SELECT_RF_BLE   PE4259_SELECT_RF2

void pe4259_select(pe4259_select_e select);
