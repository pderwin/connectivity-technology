#pragma once

typedef enum {
   SETTINGS_ID_LORAWAN_STACK
} settings_id_e;

int32_t settings_read ( settings_id_e id, void *dst, uint32_t size );
int32_t settings_write( settings_id_e id, const void *src, uint32_t size );
