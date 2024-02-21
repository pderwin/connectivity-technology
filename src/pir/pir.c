#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include "led.h"
#include "smtc_modem_api.h"

#define PIR_PRIORITY 3
#define PIR_STACK_SIZE (1024)

extern void semtracker_thread_wakeup (void);

static const struct gpio_dt_spec pir_0 = GPIO_DT_SPEC_GET(DT_ALIAS(pir0), gpios);

static void pir_thread (void *p1, void *p2, void *p3)
{
   uint32_t
      last_pir    = 2, // generate message on first time through loop
      last_msg_uptime_secs = 0,
      pir,
      rc,
      uptime_secs;

   gpio_pin_configure_dt(&pir_0, GPIO_INPUT);

   while(1) {

      /*
       * Read the state of the pin.
       */
      pir = gpio_pin_get_dt(&pir_0);

      /*
       * pir will be zero when the PIR is active.
       */
      if (pir != last_pir) {

	 last_pir = pir;

	 /*
	  * Always keep the LED up to date
	  */
	 led_command(LED_RED, LED_CMD_SET, !pir);

	 /*
	  * If we have not sent a message to the host for a while, send that now.
	  */
	 uptime_secs = k_uptime_get() / 1000;

	 if ((uptime_secs - last_msg_uptime_secs) > 5) {

	    last_msg_uptime_secs = uptime_secs;

	    /*
	     * Send message to port 60 when the line drops to low.
	     */
	    if (pir == 1) {
	       rc = smtc_modem_request_uplink(0, 60, 1, "pir_0: alarm", 12);

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

K_THREAD_STACK_DEFINE(pir_stack, PIR_STACK_SIZE);
static struct k_thread pir_kthread;

void pir_init (void)
{

   k_thread_create(
      &pir_kthread,
      pir_stack,
      K_THREAD_STACK_SIZEOF(pir_stack),
      pir_thread,
      NULL,
      NULL,
      NULL,
      PIR_PRIORITY,
      0,
      K_NO_WAIT);
}
