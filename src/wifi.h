#pragma once

#ifndef min
#define min(a,b) (a < b ? a : b)
#endif

void on_wifi_event( uint8_t pending_events );
