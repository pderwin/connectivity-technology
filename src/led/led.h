#pragma once

typedef enum {
   LED_GREEN,
   LED_RED
} led_id_e;

typedef enum {
   LED_CMD_BLINK,        /* blink continuously */
   LED_CMD_BLINK_ONCE,   /* blink one time     */
   LED_CMD_SET,
} led_cmd_e;


#define led_blink(__led, __duration) led_command(__led, LED_CMD_BLINK, __duration)
#define led_off(  __led)             led_command(__led, LED_CMD_SET,   0)

void led_command (led_id_e led_id, led_cmd_e led_cmd, uint32_t arg);
void led_init    (void);
