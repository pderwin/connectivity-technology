#include <zephyr/kernel.h>
#include <string.h>

#include "accel.h"
#include "apps_modem_common.h"
#include "apps_utilities.h"
#include "mw_assert.h"
#include "pe4259.h"
#include "smtc_board.h"
#include "smtc_hal_dbg_trace.h"
#include "smtc_modem_api.h"
#include "tracker_utility.h"
#include "wifi_middleware.h"
#include "wifi.h"

#define WIFI_PRIORITY   (4)
#define WIFI_STACK_SIZE (1024)

#define LED_PERIOD_MS 100

/*!
 * @brief Stack identifier
 */
static uint8_t stack_id = 1;

static void create_wifi_location_packet(uint8_t *packet, uint32_t packet_size, wifi_mw_event_data_scan_done_t *wifi_scan_results);

/*!
 * @brief Modem radio
 */
extern ralf_t
    *tracker_modem_radio;
extern tracker_ctx_t
    tracker_ctx;

// #define LED_W_NODE DT_ALIAS(ledw)
// static const struct gpio_dt_spec led_w = GPIO_DT_SPEC_INST_GET(LED_W_NODE, gpios);

static uint32_t tracker_app_is_tracker_in_static_mode (void)
{
   return false;
}

void wifi_thread (void *p1, void *p2, void *p3)
{
   mw_version_t
      mw_version;
   smtc_modem_return_code_t
      rc;

   (void) p1;
   (void) p2;
   (void) p3;

   /*
    * Wait for link to come up
    */
   do {
      rc =  smtc_modem_lorawan_request_link_check(0);

      k_sleep(K_MSEC(1000));

   } while (rc);

   wifi_mw_get_version( &mw_version );
   printk( "Initializing Wi-Fi middleware v%d.%d.%d\n", mw_version.major, mw_version.minor,
           mw_version.patch );

   /* Initialize Wi-Fi middleware */
   wifi_mw_init( tracker_modem_radio, stack_id );

   while (1) {

      if (accel_has_moved()) {
         /*
          * Perform the wifi scan.
          */
         printk( "Start Wi-Fi scan\n" );
         wifi_mw_scan_start( 0 );
      }

      /*
       * Pace the WiFi scan at no quicker than every 30 seconds.
       */
      k_sleep(K_MSEC(30000));
   }
}

void on_wifi_event( uint8_t pending_events )
{
   char
      wifi_data[64];
   smtc_modem_return_code_t
      rc;

   printk("%s: pending: %x \n", __func__, pending_events );

   /* Parse events */

   /*
    * See if we have WiFi data to send to uplink.
    */
   if( wifi_mw_has_event( pending_events, WIFI_MW_EVENT_SCAN_DONE ) ) {

      wifi_mw_event_data_scan_done_t
         wifi_scan_results;

      HAL_DBG_TRACE_INFO( "Wi-Fi middleware event - SCAN DONE\n" );

      wifi_mw_get_event_data_scan_done( &wifi_scan_results );

      /*
       * Create payload to send up to port 197 for location services.
       */
      if (wifi_scan_results.nbr_results) {

         create_wifi_location_packet(wifi_data, sizeof(wifi_data), &wifi_scan_results);

         /*
          * Send this payload to port 197
          */
         rc = smtc_modem_request_uplink(0, 197, 1, wifi_data, sizeof(wifi_data));
         printk("%s %d uplink rc: %d size: %d \n", __func__, __LINE__, rc, sizeof(wifi_data) );
      }
      else {
         printk("No Wifi APs found\n");
      }


      wifi_mw_display_results( &wifi_scan_results );

      /* Convert GPS timestamp to UTC timestamp */
      wifi_scan_results.timestamp = apps_modem_common_convert_gps_to_utc_time( wifi_scan_results.timestamp );

      /* Store the consumption */
      tracker_ctx.wifi_scan_charge_uAh += wifi_scan_results.power_consumption_uah;

      if( tracker_ctx.internal_log_enable )
      {
         tracker_store_wifi_in_internal_log( &wifi_scan_results );
      }
   }



   if( wifi_mw_has_event( pending_events, WIFI_MW_EVENT_TERMINATED ) )
   {
      int32_t duty_cycle_status_ms = 0;

      HAL_DBG_TRACE_INFO( "Wi-Fi middleware event - TERMINATED\n" );
      wifi_mw_get_event_data_terminated( &tracker_ctx.wifi_nb_scan_sent );
      HAL_DBG_TRACE_PRINTF( "-- number of scans sent: %u\n", tracker_ctx.wifi_nb_scan_sent.nb_scans_sent );

      ASSERT_SMTC_MODEM_RC( smtc_modem_get_duty_cycle_status( &duty_cycle_status_ms ) );
      HAL_DBG_TRACE_PRINTF( "Remaining duty cycle %d ms\n", duty_cycle_status_ms );

      /* Led start for user notification */
      smtc_board_led_pulse( smtc_board_get_led_tx_mask( ), true, LED_PERIOD_MS );
   }



   if( wifi_mw_has_event( pending_events, WIFI_MW_EVENT_SCAN_CANCELLED ) )
   {
      HAL_DBG_TRACE_INFO( "Wi-Fi middleware event - SCAN CANCELLED\n" );
   }

   if( wifi_mw_has_event( pending_events, WIFI_MW_EVENT_ERROR_UNKNOWN ) )
   {
      HAL_DBG_TRACE_INFO( "Wi-Fi middleware event - UNEXPECTED ERROR\n" );
   }

   if( wifi_mw_has_event( pending_events, WIFI_MW_EVENT_TERMINATED ) ||
       wifi_mw_has_event( pending_events, WIFI_MW_EVENT_ERROR_UNKNOWN ) )
   {
      uint32_t sequence_duration_sec = apps_modem_common_get_utc_time( ) - tracker_ctx.start_sequence_timestamp;

      if( ( tracker_ctx.wifi_nb_scan_sent.nb_scans_sent == 0 ) ||
          ( tracker_app_is_tracker_in_static_mode( ) == true ) )
      {
         HAL_DBG_TRACE_MSG( "No scan results good enough or keep alive frame, sensors values\n" );
         /* Send sensors values */
// PHIL            tracker_app_read_and_send_sensors( );
      }

      if( tracker_app_is_tracker_in_static_mode( ) == true )
      {
         if( sequence_duration_sec > tracker_ctx.static_scan_interval )
         {
            sequence_duration_sec = tracker_ctx.static_scan_interval;
         }

         /* Stop Hall Effect sensors while the tracker is static */
// PHIL            smtc_board_hall_effect_enable( false );

         HAL_DBG_TRACE_MSG( "Switch static mode\n" );
         gnss_mw_scan_aggregate( true );
         gnss_mw_scan_start( GNSS_MW_MODE_STATIC, tracker_ctx.static_scan_interval - sequence_duration_sec );
      }
      else
      {
         if( sequence_duration_sec > tracker_ctx.mobile_scan_interval )
         {
            sequence_duration_sec = tracker_ctx.mobile_scan_interval;
         }

         HAL_DBG_TRACE_MSG( "Continue in mobile mode\n" );
         gnss_mw_scan_aggregate( false );
         gnss_mw_scan_start( GNSS_MW_MODE_MOBILE, tracker_ctx.mobile_scan_interval - sequence_duration_sec );
      }
   }

   wifi_mw_clear_pending_events( );
}

static void create_wifi_location_packet(uint8_t *packet, uint32_t packet_size, wifi_mw_event_data_scan_done_t *wifi_scan_results)
{
   uint32_t
      count = 0,
      entries,
      i;
   wifi_scan_single_result_t
      *wsr;

   /*
    * First byte is always a 1 to signify MAC values with RSSI
    */
   packet[0] = 1;
   count++;

   /*
    * How many entries can fit in the remaining packet?  Each entry is 7 bytes long.
    */
   entries = (packet_size - 1) / 7;

   /*
    * Only consume what is available.
    */
   entries = min(entries, wifi_scan_results->nbr_results);

   wsr = wifi_scan_results->results;

   /*
    * Try to fill in each of the entries.
    */
   for (i=0; i < entries; i++, wsr++) {

      /*
       * Put the RSSI in first.
       */
      packet[count] = wsr->rssi;

      /*
       * copy in the MAC
       */
      memcpy(&packet[count + 1], wsr->mac_address, 6);

      /*
       * Other info in the entry is channel and type.  Currently unused.
       */

      /*
       * Ready for next entry
       */
      count += 7;
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
