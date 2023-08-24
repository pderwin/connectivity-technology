#include <zephyr/kernel.h>
#include "pe4259.h"
#include "smtc_modem_api.h"

#define WIFI_PRIORITY   (4)
#define WIFI_STACK_SIZE (1024)

// #define LED_W_NODE DT_ALIAS(ledw)
// static const struct gpio_dt_spec led_w = GPIO_DT_SPEC_INST_GET(LED_W_NODE, gpios);

#if 0
36701.530 ms |    74.920 us | lr1110_n1  | WIFI   | READ_RESULTS              |  162 | 0306 | stat1: 06 [ CMD_DAT ]
      Type | Channel | RSSI |FrameCtl |     MAC      | PhiOffset |    TimeStamp     | PeriodBeacon
       05  |   11    |  -86 |    20   | c0ffd48770a0 |    040a   | 00000fb0d648f72c |     0064
       05  |   11    |  -90 |    20   | 204e7f2b7128 |    fc12   | 000001594eed11d6 |     0064
       05  |   12    |  -77 |    20   | 98ded07d5aed |    ff20   | 0000046a17b2e157 |     0064
       05  |   24    |  -84 |    14   | 149182fc36a5 |    f7cd   | 0000000199e94951 |     0064
       05  |   16    |  -87 |    20   | cc40d0015bdc |    fb6b   | 00000fb0d370b3dd |     0064
#endif


void wifi_thread (void *p1, void *p2, void *p3)
{
   (void) p1;
   (void) p2;
   (void) p3;
   smtc_modem_return_code_t
      rc;
#if 0
   Here is the WIFI sniff results.  I saw 5 APs.
36701.530 ms |    74.920 us | lr1110_n1  | WIFI   | READ_RESULTS              |  162 | 0306 | stat1: 06 [ CMD_DAT ]
      Type | Channel | RSSI |FrameCtl |     MAC      | PhiOffset |    TimeStamp     | PeriodBeacon
      05  |   11    |  -86 |    20   | c0ffd48770a0 |    040a   | 00000fb0d648f72c |     0064
      05  |   11    |  -90 |    20   | 204e7f2b7128 |    fc12   | 000001594eed11d6 |     0064
      05  |   12    |  -77 |    20   | 98ded07d5aed |    ff20   | 0000046a17b2e157 |     0064
      05  |   24    |  -84 |    14   | 149182fc36a5 |    f7cd   | 0000000199e94951 |     0064
      05  |   16    |  -87 |    20   | cc40d0015bdc |    fb6b   | 00000fb0d370b3dd |     0064
#endif

   char
      wifi_data[] = {
      0x01, // MAC with RSSI

#if 0
      -70,
      0x94, 0xF3, 0x92, 0x8B, 0x0D, 0xC1,

      -77,
      0xAC, 0x71, 0x2E, 0x83, 0x06, 0x01,

      -85,
      0x94, 0xF3, 0x92, 0x8A, 0x5F, 0x81,

      -45,
      0xAC, 0x71, 0x2E, 0x82, 0xEB, 0xC1,

#else

      -86,
      0xc0, 0xff, 0xd4, 0x87, 0x70, 0xa0,

      -90,
      0x20, 0x4e, 0x7f, 0x2b, 0x71, 0x28,

      -77,
      0x98, 0xde, 0xd0, 0x7d, 0x5a, 0xed,

      -84,
      0x14, 0x91, 0x82, 0xfc, 0x36, 0xa5,

      -87,
      0xcc, 0x40, 0xd0, 0x01, 0x5b, 0xdc,
#endif

   };

   /*
    * Wait for link to come up
    */
   do {
      rc =  smtc_modem_lorawan_request_link_check(0);

      k_sleep(K_MSEC(1000));

   } while (rc);

   while(1) {

      /*
       * Need to connect the antenna to the LR1110.
       */
      pe4259_select(PE4259_SELECT_RF_WIFI);

      /*
       * Perform the wifi scan.
       */

      /*
       * upload the wifi data.
       */
      rc = smtc_modem_request_uplink(0, 197, 1, wifi_data, sizeof(wifi_data));
      printk("%s %d uplink rc: %d size: %d \n", __func__, __LINE__, rc, sizeof(wifi_data) );

      k_sleep(K_MSEC(30000));
   }
}

K_THREAD_STACK_DEFINE(wifi_stack, WIFI_STACK_SIZE);
struct k_thread	wifi_thread_data;

void wifi_thread_start(void)
{
   k_tid_t tid;

   tid = k_thread_create(&wifi_thread_data, wifi_stack,
                         K_THREAD_STACK_SIZEOF(wifi_stack),
                         wifi_thread,
                         NULL, NULL,	NULL,
                         WIFI_PRIORITY, 0, K_NO_WAIT);

   k_thread_name_set(tid, "wifi");
}
