#pragma once

typedef enum {
   LED_WHITE,
   LED_BLUE,
   LED_GREEN
} led_id_e;

typedef enum {
   LED_CMD_BLINK_ONCE,
} led_cmd_e;


void led_command (led_id_e led_id, led_cmd_e led_cmd, uint32_t arg);
