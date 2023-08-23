#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/trace.h>
#include "debug_gpio.h"

static const struct device *gpio0_dev = DEVICE_DT_GET(DT_NODELABEL(gpio0));

#define GPIO_RTS  (7)   // works
// #define GPIO_ACCEL_INT2 (4)

#define GPIO_RX GPIO_RTS  // works
// #define GPIO_TX GPIO_ACCEL_INT2

/*-------------------------------------------------------------------------
 *
 * name:        debug_gpio_clear
 *
 * description: set both debug pins to low.
 *
 * input:       none
 *
 * output:      none
 *
 *-------------------------------------------------------------------------*/
void debug_gpio_clear (void)
{
   debug_gpio_rx(0);
   debug_gpio_tx(0);
}

void debug_gpio_init (void)
{
   gpio_pin_configure(gpio0_dev, GPIO_RX, GPIO_OUTPUT_ACTIVE);
#ifdef GPIO_TX
   gpio_pin_configure(gpio0_dev, GPIO_TX, GPIO_OUTPUT_ACTIVE);
#endif
   debug_gpio_clear();
}

void debug_gpio_error (void)
{
//   TRACE1(TAG_GPIO_ERROR, __builtin_return_address(0) );

   if (gpio0_dev) {
      gpio_pin_set(gpio0_dev, GPIO_RX, 1);
#ifdef GPIO_TX
      gpio_pin_set(gpio0_dev, GPIO_TX, 1);
#endif
      k_busy_wait(10);

      gpio_pin_set(gpio0_dev, GPIO_RX, 0);
#ifdef GPIO_TX
      gpio_pin_set(gpio0_dev, GPIO_TX, 0);
#endif
   }
}

void debug_gpio_rx (uint32_t state)
{
   if (gpio0_dev) {
      gpio_pin_set(gpio0_dev, GPIO_RX, state);
   }
}

void debug_gpio_tx (uint32_t state)
{
   (void) state;

#ifdef GPIO_TX
   if (gpio0_dev) {
      gpio_pin_set(gpio0_dev, GPIO_TX, state);
   }
#endif
}
