#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include "led.h"
#include "smtc_modem_api.h"

#define DRIVEWAY_SENSOR_PRIORITY   3
#define DRIVEWAY_SENSOR_STACK_SIZE (1024)

extern void semtracker_thread_wakeup (void);

static const struct gpio_dt_spec driveway_sensor = GPIO_DT_SPEC_GET(DT_ALIAS(driveway_sensor), gpios);

static void driveway_sensor_thread (void *p1, void *p2, void *p3)
{
   uint32_t
      last_ds    = 1, // generate message on first time through loop
      last_msg_uptime_secs = 0,
      ds,
      rc,
      uptime_secs;

   gpio_pin_configure_dt(&driveway_sensor, GPIO_INPUT);

   k_msleep(1);

   while(1) {

      /*
       * Read the state of the pin.
       */
      ds = gpio_pin_get_dt(&driveway_sensor);

      /*
       * pir will be zero when the PIR is active.
       */
      if (ds != last_ds) {

	 printk("DS: %d (%d) \n", ds, last_ds);

	 last_ds = ds;

	 if (ds == 0) {
	    /*
	     * Always keep the LED up to date
	     */
	    led_blink(LED_RED, 100);
	 }
	 else {
	    led_off(LED_RED);
	 }

	 /*
	  * If we have not sent a message to the host for a while, send that now.
	  */
	 uptime_secs = k_uptime_get() / 1000;

	 if ((uptime_secs - last_msg_uptime_secs) > 5) {

	    last_msg_uptime_secs = uptime_secs;

	    /*
	     * Send message to port 70 when the line drops to low.
	     */
	    if (ds == 0) {
	       rc = smtc_modem_request_uplink(0, 70, 1, "driveway_sensor_0: alarm", 24);

	       /*
		* The radio may be in sleep mode, so wake it.
		*/
	       semtracker_thread_wakeup ();
	       printk("%s %d rc: %d \n", __func__,__LINE__, rc);
	    }
	 }
      }

      k_sleep(K_MSEC(100));
   }
}

K_THREAD_STACK_DEFINE(driveway_sensor_stack, DRIVEWAY_SENSOR_STACK_SIZE);
static struct k_thread driveway_sensor_kthread;

void driveway_sensor_init (void)
{

   k_thread_create(
      &driveway_sensor_kthread,
      driveway_sensor_stack,
      K_THREAD_STACK_SIZEOF(driveway_sensor_stack),
      driveway_sensor_thread,
      NULL,
      NULL,
      NULL,
      DRIVEWAY_SENSOR_PRIORITY,
      0,
      K_NO_WAIT);
}
