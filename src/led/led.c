#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include "led_internal.h"
#include "priority.h"

#define NUMBER_LEDS    (3)

#define LED_STACK_SIZE (1024)

static void handle_msg (led_msg_t *msg);
static void __led_set  (led_t *led, uint32_t val, uint32_t duration);

static led_t leds[] = {
   {
      .id   = LED_RED,
      .gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(ledr), gpios),
   },
   {
      .id   = LED_GREEN,
      .gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(ledg), gpios),
   },
   {
      .id   = LED_ALERT,
      .gpio = GPIO_DT_SPEC_GET(DT_ALIAS(alert), gpios),
   }
};

static void gpio_configure (void);

/*
 * Define a message type.
 */
K_MSGQ_DEFINE(led_msgq, sizeof(led_msg_t), 10, 4);

static void handle_msg (led_msg_t *msg)
{
   uint32_t
      i;
   led_t
      *led;

   /*
    * Find the correct LED structure based on the id.
    */
   for (i=0, led = leds; i < ARRAY_SIZE(leds); i++, led++) {
      if (led->id == msg->id) {
	 break;
      }
   }

   /*
    * Bad input?
    */
   if (i ==  ARRAY_SIZE(leds)) {
      printk("%s: Invalid ID: 0x%x \n", __func__, msg->id );
      return;
   }

   switch(msg->cmd) {

   case LED_CMD_BLINK:

      led->state          = LED_STATE_BLINK;
      led->blink_on_time  = msg->arg0;
      led->blink_off_time = msg->arg1;

      /*
       * Turn the LED on, and set timeout for turn-off
       */
      __led_set(led, 1, led->blink_on_time);

      break;

   case LED_CMD_BLINK_ONCE:

      led->state = LED_STATE_BLINK_ONCE;
      __led_set(led, 1, msg->arg0);

      break;

   case LED_CMD_SET:

      led->state = LED_STATE_IDLE;
      __led_set(led, msg->arg0, 0);

      break;

   default:
      printk("%s: unknown command: %x \n", __func__, msg->cmd);
      return;
   }
}

/*-------------------------------------------------------------------------
 *
 * name:        handle_timeout
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void handle_timeout (void)
{
   uint32_t
      i,
      new_value,
      timeout,
      uptime;
   led_t
   *led;

   uptime = k_uptime_get();

   led = leds;

   for (i=0; i < ARRAY_SIZE(leds); i++, led++) {

      switch(led->state) {

      case LED_STATE_BLINK:

	 if (uptime >= led->timeout) {
	    /*
	     * If LED is on, then send in the off timeout value.
	     */
	    if (led->value) {
	       timeout = led->blink_off_time;
	       new_value = 0;
	    }
	    else {
	       timeout = led->blink_on_time;
	       new_value = 1;
	    }

	    __led_set(led, new_value, timeout);
	 }
	 break;

      case LED_STATE_BLINK_ONCE:

	 if (uptime >= led->timeout) {
	    led->state = LED_STATE_IDLE;
	    __led_set(led, 0, 0);
	 }
	 break;

      default:
	 break;
      }
   }
}

/*
 * Send a message to the LED thread.
 */
void led_command (led_id_e id, led_cmd_e cmd, uint32_t arg0, uint32_t arg1)
{
   led_msg_t
      msg;

   msg.id   = id;
   msg.cmd  = cmd;
   msg.arg0 = arg0;
   msg.arg1 = arg1;

   k_msgq_put(&led_msgq, &msg, K_FOREVER);
}

/*-------------------------------------------------------------------------
 *
 * name:        __led_set
 *
 * description: Change the LED state and save value in the led->value variable.
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void __led_set (led_t *led, uint32_t value, uint32_t duration)
{
   /*
    * Set current state.
    */
   gpio_pin_configure_dt(&led->gpio, GPIO_OUTPUT_ACTIVE);

   led->value     = value;
   gpio_pin_set_dt(&led->gpio, value ^ 1);

   /*
    * Set timeout for state change.
    */
   led->timeout = k_uptime_get() + duration;
}

/*-------------------------------------------------------------------------
 *
 * name:        led_thread
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void led_thread (void *p1, void *p2, void *p3)
{
   uint32_t
      rc;
   led_msg_t
      msg;

   (void) p1;
   (void) p2;
   (void) p3;

   /*
    * Configure all the GPIOs
    */
   gpio_configure();

   while(1) {

      /*
       * Wait for a message, or a timeout
       */
      rc = k_msgq_get(&led_msgq, &msg, K_MSEC(50));

      if (rc == 0) {
	 handle_msg(&msg);
      }

      /*
       * This may get skewed a little because of messages coming in, but
       * not worried about that kind of resolution on an LED.
       */
      else {
	 handle_timeout();
      }
   }
}

static void gpio_configure (void)
{
   uint32_t
      i;
   led_t
      *led;

   led = leds;

   for (i=0; i< ARRAY_SIZE(leds); i++, led++) {
      gpio_pin_configure_dt(&led->gpio, GPIO_OUTPUT_ACTIVE);
      __led_set(led, 0, 0);
   }
}

K_THREAD_STACK_DEFINE(led_stack, LED_STACK_SIZE);
struct k_thread led_thread_data;

/*-------------------------------------------------------------------------
 *
 * name:        led_init
 *
 * description: Startup LED thread to handle our two LEDs.
 *
 * input:       none
 *
 * output:      none
 *
 *-------------------------------------------------------------------------*/
void led_init (void)
{
   k_tid_t tid;

   tid = k_thread_create(&led_thread_data, led_stack,
			 K_THREAD_STACK_SIZEOF(led_stack),
			 led_thread,
			 NULL, NULL,    NULL,
			 PRIORITY_LED, 0, K_NO_WAIT);

   k_thread_name_set(tid, "led");
}
