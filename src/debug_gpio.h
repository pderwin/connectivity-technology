#pragma once

#include <zephyr/kernel.h>

void debug_gpio_init (void);
void debug_gpio_rx   (uint32_t state);
void debug_gpio_tx   (uint32_t state);
