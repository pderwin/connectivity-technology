#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include "temperature_humidity.h"
#include "led.h"
#include "lis2dh.h"  // chip bit definitions from Zephyr source tree

#define TEMPERATURE_HUMIDITY_PRIORITY 3
#define TEMPERATURE_HUMIDITY_STACK_SIZE (1024)

static const struct device       *th_device = DEVICE_DT_GET(DT_ALIAS(temperature_humidity0));

static void set_configuration      (const struct device *device);
static void set_full_scale         (const struct device *device);
static void set_sampling_frequency (const struct device *device);
static void set_threshold          (const struct device *device);

static void temperature_humidity_thread (void *p1, void *p2, void *p3)
{
   struct sensor_value
      humidity_value,
      temperature_value;

   /*
    * Change the sampling frequency
    */
   set_sampling_frequency(th_device);

   /*
    * Set the sampling range
    */
   set_full_scale(th_device);

   /*
    * Raise the interrupt threshold up a bit.
    */
   set_threshold(th_device);

   /*
    * Set the interrupt filter
    */
   set_configuration(th_device);

#if 1
   {
      uint32_t
	 rc;

      while(1) {

	 rc = sensor_sample_fetch(th_device);

	 /*
	  * Read temperature and humidity
	  */
	 sensor_channel_get(th_device, SENSOR_CHAN_AMBIENT_TEMP, &temperature_value);
	 sensor_channel_get(th_device, SENSOR_CHAN_HUMIDITY,     &humidity_value);

	 printk("T: %d %d H: %d %d \n",
		temperature_value.val1, // whole number
		temperature_value.val2, // fraction

		humidity_value.val1,
		humidity_value.val2);

	 k_sleep(K_MSEC(60000));
      }
   }
#endif
}

static void set_configuration (const struct device *device)
{
   struct sensor_value sensor_value = {
      .val1 = LIS2DH_HPIS1_EN_BIT
   };

   sensor_attr_set(th_device,
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

   sensor_attr_set(th_device,
		   SENSOR_CHAN_ACCEL_XYZ,
		   SENSOR_ATTR_FULL_SCALE,
		   &sensor_value);
}

static void set_sampling_frequency (const struct device *device)
{
   struct sensor_value sensor_value = {
      .val1 = 100  // sample at 100 Hz
   };

   sensor_attr_set(th_device,
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


K_THREAD_STACK_DEFINE(temperature_humidity_stack, TEMPERATURE_HUMIDITY_STACK_SIZE);
static struct k_thread temperature_humidity_kthread;

void temperature_humidity_init (void)
{
   k_thread_create(
      &temperature_humidity_kthread,
      temperature_humidity_stack,
      K_THREAD_STACK_SIZEOF(temperature_humidity_stack),
      temperature_humidity_thread,
      NULL,
      NULL,
      NULL,
      TEMPERATURE_HUMIDITY_PRIORITY,
      0,
      K_NO_WAIT);
}
