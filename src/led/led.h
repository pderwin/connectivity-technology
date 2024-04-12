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


#define led_blink(__led, __on_time, __off_time) led_command(__led, LED_CMD_BLINK,      __on_time, __off_time)
#define led_blink_once(__led, __on_time)        led_command(__led, LED_CMD_BLINK_ONCE, __on_time, 0)
#define led_off(  __led)                        led_set(__led, 0)
#define led_on(  __led)                         led_set(__led, 1)
#define led_set(  __led, __value)               led_command(__led, LED_CMD_SET,     __value, 0)

void led_command (led_id_e led_id, led_cmd_e led_cmd, uint32_t arg0, uint32_t arg1);
void led_init    (void);
