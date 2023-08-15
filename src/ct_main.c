#include <zephyr/kernel.h>
#include <zephyr/init.h>
#include "accel.h"

int ct_main(const struct device *device)
{
   (void) device;

   printk("%s: initializing...\n", __func__);

   accel_init();

   printk("%s: initialization complete\n", __func__);

   return 0;
}


SYS_INIT(ct_main, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);
