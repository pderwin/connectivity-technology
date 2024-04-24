#include <zephyr/kernel.h>
#include "buttons_internal.h"
#include "priority.h"
#include "semtracker.h"

#define BUTTONS_STACK_SIZE (1024)

#define CNT_DEBOUNCE (5)

static button_t
    btn1 = {
       .gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(btn1), gpios),
       .id   = 1
       },
    btn2 = {
       .gpio = GPIO_DT_SPEC_GET(DT_NODELABEL(btn2), gpios),
       .id   = 2
    };

static void btn_init (button_t *btn);
static void button_irq_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins);

/*-------------------------------------------------------------------------
 *
 * name:        btn_init
 *
 * description: Setup the GPIOs for input for each button
 *
 * input:       none
 *
 * output:      none
 *
 *-------------------------------------------------------------------------*/
static void btn_init (button_t *btn)
{
   gpio_pin_configure_dt(&btn->gpio, GPIO_INPUT);

   gpio_pin_interrupt_configure_dt(&btn->gpio, GPIO_INT_EDGE_RISING);

   gpio_init_callback(&btn->gpio_callback, button_irq_callback, BIT(btn->gpio.pin));

   if (gpio_add_callback_dt(&btn->gpio, &btn->gpio_callback) < 0) {
      printk("%s: Could not set GPIO callback", __func__);
      return;
   }
}

static void button_irq_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
   if (pins) {
      semtracker_cmd(SEMTRACKER_CMD_WIFI_SCAN, pins);
   }
}


/*-------------------------------------------------------------------------
 *
 * name:        buttons_init
 *
 * description: Create button thread
 *
 * input:       none
 *
 * output:      none
 *
 *-------------------------------------------------------------------------*/
void buttons_init (void)
{
   /*
    * Configure all the GPIOs
    */
   btn_init(&btn1);
   btn_init(&btn2);
}
