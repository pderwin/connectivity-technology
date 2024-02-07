#pragma once

typedef enum {
   LED_GREEN,
   LED_RED
} led_id_e;

typedef enum {
   LED_CMD_BLINK,        /* blink continuously */
   LED_CMD_BLINK_ONCE,   /* blink one time     */
} led_cmd_e;


void led_command (led_id_e led_id, led_cmd_e led_cmd, uint32_t arg);
