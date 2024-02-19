#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include "accel.h"
#include "driveway_sensor.h"
#include "led.h"
#include "pir.h"
#include "temperature_humidity.h"

void bt_thread_start(void);
void semtracker_thread_start (void);
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
   printk("%s: initializing...\n", __func__);

   driveway_sensor_init();

   led_init();

   bt_thread_start();

   semtracker_thread_start();

   accel_init();

   temperature_humidity_init();

   /*
    * Startup the PIR sensor
    */
   pir_init();

   /*
    * Make the green LED blink until further notice.
    */
   led_command(LED_GREEN, LED_CMD_BLINK, 1000);

   printk("%s: initialization complete\n", __func__);

   return 0;
}

SYS_INIT(ct_main, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
