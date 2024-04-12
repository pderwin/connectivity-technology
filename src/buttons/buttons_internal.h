#pragma once

#include <zephyr/drivers/gpio.h>
#include "buttons.h"

typedef struct {

   uint32_t
       id;    /* 1 or 2 */
   uint32_t
       pin;
   struct gpio_dt_spec
       gpio;
   struct gpio_callback
       gpio_callback;

} button_t;
