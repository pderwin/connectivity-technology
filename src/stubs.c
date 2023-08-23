#include <zephyr/kernel.h>
#include <stdint.h>

#define STUB(__subr) uint32_t __subr() { /* printk("STUB: %s called (from %p)\n", #__subr, __builtin_return_address(0) ); */ return 0; }

STUB(hal_mcu_reset);
STUB(mcu_panic);
STUB(smtc_modem_hal_reset_mcu);
STUB(smtc_modem_hal_radio_irq_clear_pending );
STUB(hal_mcu_enable_irq);
STUB(hal_mcu_get_vref_level);
STUB(hal_mcu_delay_ms);
STUB(hal_mcu_get_tick);
STUB(hal_mcu_partial_sleep_enable);

void hal_mcu_set_sleep_for_ms (uint32_t mSecs)
{
   k_sleep(K_MSEC(mSecs));
}
