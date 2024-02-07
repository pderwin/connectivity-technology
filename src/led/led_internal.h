#pragma once

#include "led.h"


typedef struct {
   led_cmd_e cmd;
   led_id_e  id;
   uint32_t  arg;
} msg_t;

/*---------------------- */

typedef enum {
   LED_STATE_IDLE,
   LED_STATE_BLINK,
   LED_STATE_BLINK_ONCE,

} led_state_e;

typedef struct {

   led_id_e
       id;

   struct gpio_dt_spec
       gpio;

   led_state_e
       state;
   uint32_t
       blink_rate;  // timeout for each blink of the LED.  Set from msg->arg.

   uint32_t
       timeout;  // value computed that will be the uptime when the next transition occurs.
   uint32_t
       value;   // Current LED value

} led_t;
