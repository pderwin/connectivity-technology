#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include "accel.h"

void bt_thread_start(void);
void led_thread_start(void);
void semtracker_thread_start (void);
void wifi_thread_start (void);

int ct_main(const struct device *device)
{
   (void) device;

   printk("%s: initializing...\n", __func__);

   bt_thread_start();
   led_thread_start();

   semtracker_thread_start();

//   accel_init();

   wifi_thread_start();

   printk("%s: initialization complete\n", __func__);

   return 0;
}


SYS_INIT(ct_main, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
