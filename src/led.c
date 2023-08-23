#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>

#define LED_PRIORITY   (4)
#define LED_STACK_SIZE (1024)

static const struct gpio_dt_spec led_w = GPIO_DT_SPEC_GET(DT_NODELABEL(ledw), gpios);

void led_thread (void *p1, void *p2, void *p3)
{
   (void) p1;
   (void) p2;
   (void) p3;

   gpio_pin_configure_dt(&led_w, GPIO_OUTPUT_ACTIVE);

   while(1) {
      gpio_pin_set_dt(&led_w, 1);
      k_sleep(K_MSEC(500));

      gpio_pin_set_dt(&led_w, 0);
      k_sleep(K_MSEC(1000));
   }
}

K_THREAD_STACK_DEFINE(led_stack, LED_STACK_SIZE);
struct k_thread	led_thread_data;

void led_thread_start(void)
{
   k_tid_t tid;

   tid = k_thread_create(&led_thread_data, led_stack,
                         K_THREAD_STACK_SIZEOF(led_stack),
                         led_thread,
                         NULL, NULL,	NULL,
                         LED_PRIORITY, 0, K_NO_WAIT);

   k_thread_name_set(tid, "led");
}
