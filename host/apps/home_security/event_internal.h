#pragma once

#include <stdint.h>
#include "event.h"

typedef struct {

   uint32_t
       f_port;
   char
       payload[80];

} event_t;
