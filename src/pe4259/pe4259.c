#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/trace.h>
#include <zephyr/devicetree.h>
#include "pe4259.h"

static const struct gpio_dt_spec ctrl = GPIO_DT_SPEC_GET(DT_NODELABEL(pe4259ctrl), gpios);
static const struct gpio_dt_spec vdd  = GPIO_DT_SPEC_GET(DT_NODELABEL(pe4259vdd),  gpios);

void pe4259_select(pe4259_select_e select)
{
   uint32_t
      ctrl_val,
      vdd_val;

   switch(select) {
       case PE4259_SELECT_RF1:
          ctrl_val = 1;
          vdd_val  = 0;
          break;
       case PE4259_SELECT_RF2:
          ctrl_val = 0;
          vdd_val  = 1;
          break;
       default:
          printk("%s: Error -- invalid selection: %x from %p \n", __func__, select,__builtin_return_address(0) );
          return;
   }

   TRACE2(TAG_PE4259_SELECT, ctrl_val, vdd_val);

   gpio_pin_set_dt(&ctrl, ctrl_val);
   gpio_pin_set_dt(&vdd,  vdd_val);
}


void pe4259_init (void)
{
    gpio_pin_configure_dt(&ctrl, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&vdd,  GPIO_OUTPUT_INACTIVE);
}
