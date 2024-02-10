#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include "accel.h"
#include "led.h"

void bt_thread_start(void);
void led_thread_start(void);
void semtracker_thread_start (void);
void temperature_humidity_init (void);
void wifi_thread_start (void);

int ct_main(void)
{
   printk("%s: initializing...\n", __func__);

   bt_thread_start();
   led_thread_start();

   semtracker_thread_start();

   accel_init();

   temperature_humidity_init();

   /*
    * Make the green LED blink until further notice.
    */
   led_command(LED_GREEN, LED_CMD_BLINK, 1000);

   printk("%s: initialization complete\n", __func__);

   return 0;
}

SYS_INIT(ct_main, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
