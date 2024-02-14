#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/trace.h>
#include "debug_gpio.h"

#if 0

static const struct gpio_dt_spec debug_gpio_0 = GPIO_DT_SPEC_GET(DT_NODELABEL(debug_gpio_0), gpios);
#endif

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
#if 0
   debug_gpio_rx(0);
   debug_gpio_tx(0);
#endif
}

void debug_gpio_init (void)
{
#if 0

//   gpio_pin_configure_dt(&debug_gpio_0, GPIO_OUTPUT_ACTIVE);
#ifdef GPIO_TX
   gpio_pin_configure_dt(gpio0_dev, GPIO_OUTPUT_ACTIVE);
#endif
   debug_gpio_clear();
#endif
}

void debug_gpio_error (void)
{
#if 0
//   TRACE1(TAG_GPIO_ERROR, __builtin_return_address(0) );

   gpio_pin_set_dt(&debug_gpio_0, 1);
#ifdef GPIO_TX
   gpio_pin_set(gpio0_dev, GPIO_TX, 1);
#endif
   k_busy_wait(10);

   gpio_pin_set_dt(&debug_gpio_0, 0);
#ifdef GPIO_TX
   gpio_pin_set(gpio0_dev, GPIO_TX, 0);
#endif
#endif

}

void debug_gpio_rx (uint32_t state)
{
#if 0
   gpio_pin_set_dt(&debug_gpio_0, state);
#endif
}

void debug_gpio_tx (uint32_t state)
{
#if 0
   (void) state;

#ifdef GPIO_TX
   if (gpio0_dev) {
      gpio_pin_set(gpio0_dev, GPIO_TX, state);
   }
#endif
#endif
}
