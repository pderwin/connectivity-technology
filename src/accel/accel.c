#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include "accel.h"
#include "lis2dh.h"  // chip bit definitions from Zephyr source tree

#define ACCEL_PRIORITY 3
#define ACCEL_STACK_SIZE (1024)

static const struct device       *accel_device = DEVICE_DT_GET(DT_ALIAS(accel0));

static const struct gpio_dt_spec vdd_sensor_en = GPIO_DT_SPEC_INST_GET(0, vdd_sensor_en_gpios );

static void set_configuration      (const struct device *device);
static void set_full_scale         (const struct device *device);
static void set_sampling_frequency (const struct device *device);
static void set_threshold          (const struct device *device);

static struct sensor_trigger accel_trigger =
{
   .type = SENSOR_TRIG_DELTA
};

static void accel_callback ()
{
   uint32_t
      rc;
   struct sensor_value
      x_sensor_value,
      y_sensor_value,
      z_sensor_value;

   rc = sensor_sample_fetch(accel_device);

   sensor_channel_get(accel_device, SENSOR_CHAN_ACCEL_X, &x_sensor_value);
   sensor_channel_get(accel_device, SENSOR_CHAN_ACCEL_Y, &y_sensor_value);
   sensor_channel_get(accel_device, SENSOR_CHAN_ACCEL_Z, &z_sensor_value);

   printk("X: %d.%d Y: %d.%d z: %d.%d\n",
          x_sensor_value.val1,
          x_sensor_value.val2,

          y_sensor_value.val1,
          y_sensor_value.val2,

          z_sensor_value.val1,
          z_sensor_value.val2);
}

static void accel_thread (void *p1, void *p2, void *p3)
{
   uint32_t
      rc;

   /*
    * Change the sampling frequency
    */
   set_sampling_frequency(accel_device);

   /*
    * Set the sampling range
    */
   set_full_scale(accel_device);

   /*
    * Raise the interrupt threshold up a bit.
    */
   set_threshold(accel_device);

   /*
    * Set the interrupt filter
    */
   set_configuration(accel_device);

   /*
    * Set trigger callback.
    */
   sensor_trigger_set(accel_device, &accel_trigger, accel_callback);


   while(1) {
      printk("%s: fetch \n", __func__);

      rc = sensor_sample_fetch(accel_device);
      printk("sample rc: %d \n", rc);

      k_sleep(K_MSEC(5000));
   }
}

static void set_configuration (const struct device *device)
{
   struct sensor_value sensor_value = {
      .val1 = LIS2DH_HPIS1_EN_BIT
   };

   sensor_attr_set(accel_device,
                   SENSOR_CHAN_ACCEL_XYZ,
                   SENSOR_ATTR_CONFIGURATION,
                   &sensor_value);
}
static void set_full_scale (const struct device *device)
{
   struct sensor_value sensor_value = {
      .val1 = 0,
      .val2 = (2 * SENSOR_G)
   };

   sensor_attr_set(accel_device,
                   SENSOR_CHAN_ACCEL_XYZ,
                   SENSOR_ATTR_FULL_SCALE,
                   &sensor_value);
}

static void set_sampling_frequency (const struct device *device)
{
   struct sensor_value sensor_value = {
      .val1 = 100  // sample at 100 Hz
   };

   sensor_attr_set(accel_device,
                   SENSOR_CHAN_ACCEL_XYZ,
                   SENSOR_ATTR_SAMPLING_FREQUENCY,
                   &sensor_value);
}

static void set_threshold (const struct device *device)
{
   /*
    * we're on a 4g scale, so each tick seems to be 32 mg.  Just pull
    * a sample number out of the air to get going.
    */
   struct sensor_value sensor_value = {
      .val1 = 0,
      .val2 = SENSOR_G / 8
   };

   sensor_attr_set(device,
                   SENSOR_CHAN_ACCEL_XYZ,
                   SENSOR_ATTR_SLOPE_TH,
                   &sensor_value);
}


K_THREAD_STACK_DEFINE(accel_stack, ACCEL_STACK_SIZE);
static struct k_thread accel_kthread;

void accel_init (void)
{
   k_thread_create(
      &accel_kthread,
      accel_stack,
      K_THREAD_STACK_SIZEOF(accel_stack),
      accel_thread,
      NULL,
      NULL,
      NULL,
      ACCEL_PRIORITY,
      0,
      K_NO_WAIT);
}

int sigsense_power_up (const struct device *device)
{
   /*
    * Need to configure the VDD_SENSOR_EN gpio and drive it high to turn power on to the sense card.
    */
   gpio_pin_configure_dt(&vdd_sensor_en, GPIO_OUTPUT_ACTIVE);
   gpio_pin_set_dt(&vdd_sensor_en, 1);

   return 0;
}

SYS_INIT(sigsense_power_up, POST_KERNEL, CONFIG_I2C_INIT_PRIORITY);
