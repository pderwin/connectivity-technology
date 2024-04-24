#pragma once

#include <stdint.h>

void downlink_parse( uint8_t f_port, const uint8_t* payload, uint8_t length);
