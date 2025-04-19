#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include "led.h"
#include "priority.h"
#include "smtc_modem_api.h"

#define DRIVEWAY_SENSOR_PRIORITY   3
#define DRIVEWAY_SENSOR_STACK_SIZE (1024)

extern void semtracker_thread_wakeup (void);

static const struct gpio_dt_spec driveway_sensor = GPIO_DT_SPEC_GET(DT_ALIAS(driveway_sensor), gpios);

void driveway_sensor_alarm (void)
{
   static char
      *alarm_str = "drvw: alrm";
   uint32_t
      rc;

   rc = smtc_modem_request_uplink(0,                   // stack_id
				  70,                  // f_port
				  1,                   // confirmed,
				  alarm_str,           // payload
				  strlen(alarm_str) ); // payload length
   printk("%s %d rc: %d \n", __func__,__LINE__, rc);


   /*
    * The radio may be in sleep mode, so wake it.
    */
   semtracker_thread_wakeup ();
}

static void driveway_sensor_thread (void *p1, void *p2, void *p3)
{
   uint32_t
      last_ds    = 1, // generate message on first time through loop
      last_msg_uptime_secs = 0,
      ds,
      uptime_secs;

   gpio_pin_configure_dt(&driveway_sensor, GPIO_INPUT);

   k_msleep(1);

   while(1) {

      /*
       * Read the state of the pin.
       */
      ds = gpio_pin_get_dt(&driveway_sensor);

      /*
       * value will be zero when the sensor is active.
       */
      if (ds != last_ds) {

	 last_ds = ds;

	 led_set(LED_RED, (ds == 0) ? 1 : 0);

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

	       driveway_sensor_alarm();

	       /*
		* The radio may be in sleep mode, so wake it.
		*/
	       semtracker_thread_wakeup ();
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
      PRIORITY_DRIVEWAY_SENSOR,
      0,
      K_NO_WAIT);
}
