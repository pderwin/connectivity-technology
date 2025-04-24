#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include <zephyr/version.h>
#include "accel.h"
#include "buttons.h"
#include "driveway_sensor.h"
#include "led.h"
#include "pir.h"
#include "semtracker.h"
#include "temperature_humidity.h"

void bluetooth_init(void);
void wifi_thread_start (void);

/*-------------------------------------------------------------------------
 *
 * name:        ct_main
 *
 * description: connectivity-technology entry point via SYS_INIT.  This
 *              implies it is running on the 'main' thread.  Kick off any
 *              other threads that we need.
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
int ct_main(void)
{
   printk("Tracker Init\n");
   printk("Zephyr version: %s\n", KERNEL_VERSION_STRING);

   driveway_sensor_init();

   led_init();

   buttons_init();

#if CONFIG_BT
   bluetooth_init();
#endif

   semtracker_init();

   accel_init();

   temperature_humidity_init();

   /*
    * Startup the PIR sensor
    */
   pir_init();

   /*
    * Make the green LED blink until further notice.
    */
   led_on(LED_GREEN);

   return 0;
}

SYS_INIT(ct_main, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
