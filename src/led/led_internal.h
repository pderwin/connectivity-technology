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
   LED_STATE_BLINK_ONCE,
   LED_STATE_BLINK_REPEAT,

} led_state_e;

typedef struct {

   led_id_e
       id;

   struct gpio_dt_spec
       gpio;

   led_state_e
       state;

   uint32_t
       timeout;
   uint32_t
       val;   // selected state via message

} led_t;
