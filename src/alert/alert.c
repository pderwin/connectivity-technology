#include <zephyr/kernel.h>
#include "alert.h"
#include "led.h"

/*-------------------------------------------------------------------------
 *
 * name:        alert_set
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
void alert_set (uint32_t value)
{
   if (value) {
      printk("%s %d LED BLINK\n", __func__,__LINE__);
      led_blink(LED_ALERT, 100, 2000);
   }
   else {
      printk("%s %d LED OFF\n", __func__,__LINE__);

      led_off(LED_ALERT);
   }
}
