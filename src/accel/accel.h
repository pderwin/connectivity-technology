#pragma once

void accel_init (void);


#include <zephyr/types.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/lora.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/device.h>
#include "lr11xx_hal.h"

#if DT_HAS_COMPAT_STATUS_OKAY(semtech_lr1110)
#define DT_DRV_COMPAT semtech_lr1110
#define LR1110_DEVICE_ID LR1110
#else
#error No LR1110 instance in device tree.
#endif

#define HAVE_GPIO_CS		DT_INST_SPI_DEV_HAS_CS_GPIOS(0)
#define HAVE_GPIO_ANTENNA_ENABLE			\
        DT_INST_NODE_HAS_PROP(0, antenna_enable_gpios)
#define HAVE_GPIO_TX_ENABLE	DT_INST_NODE_HAS_PROP(0, tx_enable_gpios)
#define HAVE_GPIO_RX_ENABLE	DT_INST_NODE_HAS_PROP(0, rx_enable_gpios)
#define HAVE_GPIO_RADIO_ENABLE	DT_INST_NODE_HAS_PROP(0, radio_enable_gpios)

#define GPIO_CS_LABEL		DT_INST_SPI_DEV_CS_GPIOS_LABEL(0)
#define GPIO_CS_PIN		DT_INST_SPI_DEV_CS_GPIOS_PIN(0)
#define GPIO_CS_FLAGS		DT_INST_SPI_DEV_CS_GPIOS_FLAGS(0)

#define GPIO_ANTENNA_ENABLE_PIN	DT_INST_GPIO_PIN(0, antenna_enable_gpios)
#define GPIO_TX_ENABLE_PIN	DT_INST_GPIO_PIN(0, tx_enable_gpios)
#define GPIO_RX_ENABLE_PIN	DT_INST_GPIO_PIN(0, rx_enable_gpios)
#define GPIO_RADIO_ENABLE_PIN	DT_INST_GPIO_PIN(0, radio_enable_gpios)

struct lr11xx_config {
        struct spi_dt_spec bus;
#if HAVE_GPIO_ANTENNA_ENABLE
        struct gpio_dt_spec antenna_enable;
#endif
#if HAVE_GPIO_TX_ENABLE
        struct gpio_dt_spec tx_enable;
#endif
#if HAVE_GPIO_RX_ENABLE
        struct gpio_dt_spec rx_enable;
#endif
#if HAVE_GPIO_RADIO_ENABLE
        struct gpio_dt_spec radio_enable;
#endif
};

struct lr11xx_data {
   struct gpio_callback dio1_irq_callback;
   struct k_work dio1_irq_work;

   void (*radio_dio_irq_callback)(void *context);
   void *radio_dio_irq_context;
};

void lr11xx_reset(struct lr11xx_data *dev_data);
void lr11xx_reset_programming_mode(struct lr11xx_data *dev_data);
void lr11xx_CS_wakeup(struct lr11xx_data *dev_data);

bool lr11xx_is_busy(struct lr11xx_data *dev_data);

uint32_t lr11xx_get_dio1_pin_state(struct lr11xx_data *dev_data);

void lr11xx_dio1_irq_enable(struct lr11xx_data *dev_data);

void lr11xx_dio1_irq_disable(struct lr11xx_data *dev_data);

int lr11xx_variant_init(const struct device *dev);
