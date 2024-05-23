/*
 * Zephyr includes
 */
#include <zephyr/kernel.h>

/*
 * Semtech includes.
 */
#include "apps_modem_event.h"
#include "lr11xx_firmware_update.h"
#include "lr11xx_drv.h"
#include "lr11xx_system.h"
#include "smtc_board.h"
#include "smtc_board_ralf.h"
#include "smtc_hal_dbg_trace.h"
#include "smtc_modem_utilities.h"
#include "smtc_modem_api_str.h"
#include "smtc_modem_geolocation_api.h"

/*
 * local includes.
 */
#include "config.h"
#include "downlink.h"
#include "led.h"
#include "lr11xx_firmware.h"
#include "priority.h"
#include "semtracker_internal.h"

/*
 * Our configuation structure
 */
static config_t config;

static uint8_t
    stack_id = 0;

/*
 * Define a message Queue
 */
K_MSGQ_DEFINE(semtracker_msgq, sizeof(semtracker_msg_t), 10, 4);

static void configure_lorawan_parms( void );
static void display_lbm_version    ( void );
static void init_config            ( void );
static void semtracker_msg_handle  ( semtracker_msg_t *msg );
static void set_data_rate_profile  ( uint32_t region );


/*-------------------------------------------------------------------------
 *
 * name:
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void on_down_data( const uint8_t* payload, uint8_t length, smtc_modem_dl_metadata_t metadata,
			  uint8_t remaining_data_nb )
{
   printk("%s %d length: %d rem: %d \n", __func__,__LINE__, length, remaining_data_nb);

   if ( length ) {
      printk("Downlink fport: %d size: %d \n", metadata.fport, length );
      printk("rssi: %d\n", metadata.rssi);

      downlink_parse(metadata.fport, payload, length);
    }
}



/*-------------------------------------------------------------------------
 *
 * name:        on_gnss_scan_done
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void on_gnss_scan_done( smtc_modem_gnss_event_data_scan_done_t data )
{
   printf("%s %d \n", __func__,__LINE__);
#if 0
    HAL_DBG_TRACE_INFO( "on_gnss_scan_done\n" );

    /* Convert GPS timestamp to UTC timestamp */
    for( int i = 0; i < data.nb_scans_valid; i++ )
    {
	data.scans[i].timestamp = apps_modem_common_convert_gps_to_utc_time( data.scans[i].timestamp );
    }

    /* Store the consumption */
    tracker_ctx.gnss_scan_charge_nAh += data.power_consumption_nah;

    /* Store the scans duration */
    tracker_ctx.scans_duration = data.navgroup_duration_ms;

    /* timestamp the beginning ot the TX sequence */
    if( data.timestamp != 0 )
    {
	tracker_ctx.scans_timestamp = apps_modem_common_convert_gps_to_utc_time( data.timestamp );
    }
    else
    {
	tracker_ctx.scans_timestamp = apps_modem_common_get_utc_time( stack_id );
    }
    tracker_ctx.start_sequence_timestamp = tracker_ctx.scans_timestamp;

    /* Log results in internal memory */
    if( tracker_ctx.internal_log_enable )
    {
	tracker_store_gnss_in_internal_log( &data, tracker_ctx.scans_timestamp );
    }
#endif
}

/*-------------------------------------------------------------------------
 *
 * name:        on_gnss_scan_terminated
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void on_gnss_scan_terminated( smtc_modem_gnss_event_data_terminated_t data )
{
      printf("%s %d \n", __func__,__LINE__);

#if 0
   int32_t                  duty_cycle_status_ms = 0;
    smtc_modem_status_mask_t modem_status;

    ASSERT_SMTC_MODEM_RC( smtc_modem_get_duty_cycle_status( stack_id, &duty_cycle_status_ms ) );
    HAL_DBG_TRACE_PRINTF( "Remaining duty cycle %d ms\n", duty_cycle_status_ms );

    /* Led start for user notification */
    smtc_board_led_pulse( smtc_board_get_led_tx_mask( ), true, LED_PERIOD_MS );

    /* Has tracker moved ? */
    tracker_ctx.accelerometer_move_history =
	( tracker_ctx.accelerometer_move_history << 1 ) + is_accelerometer_detected_moved( );

    if( ( data.nb_scans_sent == 0 ) || ( tracker_ctx.scan_priority == TRACKER_NO_PRIORITY ) )
    {
	HAL_DBG_TRACE_MSG( "Start Wi-Fi scan\n" );
	smtc_modem_wifi_scan( stack_id, 0 );
    }
    else
    {
	if( tracker_app_is_tracker_in_static_mode( ) == true )
	{
	    /* Stop Hall Effect sensors while the tracker is static */
	    smtc_board_hall_effect_enable( false );

	    HAL_DBG_TRACE_MSG( "Switch in static mode\n" );
	    smtc_modem_gnss_scan_aggregate( stack_id, true );
	    smtc_modem_gnss_scan( stack_id, SMTC_MODEM_GNSS_MODE_STATIC, tracker_ctx.static_scan_interval );

	    /* Send sensors values in static mode */
	    smtc_modem_get_status( stack_id, &modem_status );
	    if( ( modem_status >> SMTC_MODEM_STATUS_JOINED ) == 1 )
	    {
		tracker_app_read_and_send_sensors( );
	    }
	}
	else
	{
	    HAL_DBG_TRACE_MSG( "Continue in mobile mode\n" );
	    smtc_modem_gnss_scan_aggregate( stack_id, false );
	    smtc_modem_gnss_scan( stack_id, SMTC_MODEM_GNSS_MODE_MOBILE, tracker_ctx.mobile_scan_interval );
	}
    }
#endif
}


/*-------------------------------------------------------------------------
 *
 * name:        on_joined
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void on_joined( void )
{
    /* Stop network research notification */
   led_on(LED_GREEN);

   /*
    * Looks like basically a NOP, but found in other examples.
    */
   smtc_modem_set_class( stack_id, SMTC_MODEM_CLASS_A);

   /* Set the ADR according to the region */
   set_data_rate_profile( SMTC_MODEM_REGION_US_915 );

   smtc_modem_trig_lorawan_mac_request( stack_id,
					SMTC_MODEM_LORAWAN_MAC_REQ_LINK_CHECK | SMTC_MODEM_LORAWAN_MAC_REQ_DEVICE_TIME );
}

/*-------------------------------------------------------------------------
 *
 * name:        on_link_status
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void on_link_status( smtc_modem_event_mac_request_status_t status, uint8_t margin, uint8_t gw_cnt )
{
      printf("%s %d \n", __func__,__LINE__);
}

/*-------------------------------------------------------------------------
 *
 * name:
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void on_lorawan_mac_time( smtc_modem_event_mac_request_status_t status, uint32_t gps_time_s,
				 uint32_t gps_fractional_s )
{
//    apps_modem_common_get_utc_time( stack_id );
}



/*-------------------------------------------------------------------------
 *
 * name:        on_reset
 *
 * description: Called from LBM when the radio reset occurs.
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void on_reset( )
{
   printk( "%s:\n\n", __func__ );

   if( smtc_board_is_ready( ) == true )
   {
      /* System reset */
      //    smtc_modem_hal_reset_mcu( );
   }
   else
   {
      smtc_board_set_ready( true );
   }
}

/*-------------------------------------------------------------------------
 *
 * name:        on_wifi_scan_dopne
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/

static void on_wifi_scan_done( smtc_modem_wifi_event_data_scan_done_t data )
{
   printk("%s %d (from %p) \n", __func__,__LINE__, __builtin_return_address(0) );

#if 0

   HAL_DBG_TRACE_INFO( "on_wifi_scan_done\n" );

   // Store the consumption
   tracker_ctx.wifi_scan_charge_nAh += data.power_consumption_nah;

   if( tracker_ctx.internal_log_enable )
   {
      tracker_store_wifi_in_internal_log( &data, tracker_ctx.scans_timestamp );
   }
#endif
}

/*-------------------------------------------------------------------------
 *
 * name:        on_wifi_scan_terminated
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void on_wifi_scan_terminated( smtc_modem_wifi_event_data_terminated_t data )
{
   printk("%s %d (from %p) \n", __func__,__LINE__, __builtin_return_address(0) );
#if 0
    int32_t                  duty_cycle_status_ms = 0;
    smtc_modem_status_mask_t modem_status;

    HAL_DBG_TRACE_INFO( "on_wifi_scan_terminated\n" );

    ASSERT_SMTC_MODEM_RC( smtc_modem_get_duty_cycle_status( stack_id, &duty_cycle_status_ms ) );
    HAL_DBG_TRACE_PRINTF( "Remaining duty cycle %d ms\n", duty_cycle_status_ms );

    // Led start for user notification
    smtc_board_led_pulse( smtc_board_get_led_tx_mask( ), true, LED_PERIOD_MS );

    if( ( data.nb_scans_sent == 0 ) || ( tracker_app_is_tracker_in_static_mode( ) == true ) )
    {
	HAL_DBG_TRACE_MSG( "No scan results good enough or keep alive frame, sensors values\n" );
	smtc_modem_get_status( stack_id, &modem_status );
	if( ( modem_status >> SMTC_MODEM_STATUS_JOINED ) == 1 )
	{
	    tracker_app_read_and_send_sensors( );
	}
    }

    if( tracker_app_is_tracker_in_static_mode( ) == true )
    {
	// Stop Hall Effect sensors while the tracker is static
	smtc_board_hall_effect_enable( false );

	HAL_DBG_TRACE_MSG( "Switch static mode\n" );
	smtc_modem_gnss_scan_aggregate( stack_id, true );
	smtc_modem_gnss_scan( stack_id, SMTC_MODEM_GNSS_MODE_STATIC, tracker_ctx.static_scan_interval );
    }
    else
    {
	HAL_DBG_TRACE_MSG( "Continue in mobile mode\n" );
	smtc_modem_gnss_scan_aggregate( stack_id, false );
	smtc_modem_gnss_scan( stack_id, SMTC_MODEM_GNSS_MODE_MOBILE, tracker_ctx.mobile_scan_interval );
    }
#endif

}

/*-------------------------------------------------------------------------
 *
 * name:        app_start
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void app_start ( void )
{
   HAL_DBG_TRACE_INFO( "###### ===== LoRa Basics Modem Tracker application ==== ######\n\n" );

   display_lbm_version( );

   /* Init Tracker context */
   HAL_DBG_TRACE_INFO( "###### ===== Init context ==== ######\n" );

   init_config( );

   /* Initialize LoRaWAN parameters */
   configure_lorawan_parms( );

   /* Start the BLE thread*/
//   start_ble_thread( TRACKER_ADV_TIMEOUT_MS, stack_id );

   if ( config.airplane_mode == false ) {

      /*
       * Blink red LED periodically.  One for a half-second every 5 seconds.
       */
      led_blink(LED_GREEN, 400, 2500);

      /* Start the Join process */
      smtc_modem_join_network( stack_id );
      HAL_DBG_TRACE_INFO( "###### ===== JOINING ==== ######\n\n" );

 #if defined( ADD_SMTC_STORE_AND_FORWARD )
      /* Init store and forward service */
//      smtc_modem_store_and_forward_set_state( stack_id, true );
#endif
      /* Init geolocation services */
//      smtc_modem_gnss_send_mode( stack_id, config.geolocation_send_mode );
//      smtc_modem_wifi_send_mode( stack_id, config.geolocation_send_mode );
//      smtc_modem_gnss_scan_aggregate( stack_id, false );

      /* Program GNSS scan */
//      smtc_modem_gnss_scan( stack_id, SMTC_MODEM_GNSS_MODE_MOBILE, 20 );

      /* Start almanac demodulation service */
//      smtc_modem_almanac_demodulation_set_constellations( stack_id, SMTC_MODEM_GNSS_CONSTELLATION_GPS_BEIDOU );
//      smtc_modem_almanac_demodulation_start( stack_id );
   }
   else {
      printk( "TRACKER IN AIRPLANE MODE\n\n" );

      /* Stop Hall Effect sensors while the tracker is static in airplane mode */
//      smtc_board_hall_effect_enable( false );

      /* Reset accelerometer IRQ if any */
//      is_accelerometer_detected_moved( );
   }
}

/*-------------------------------------------------------------------------
 *
 * name:        semtracker_thread
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
void semtracker_thread( void *p1, void *p2, void *p3)
{
   uint32_t
      rc,
      sleep_msecs;
   const void
      *radio_context;
   semtracker_msg_t
      msg;

   /*
    * Find our radio device driver and store as our 'context'.
    */
   radio_context = lr11xx_drv_radio_context_get();

   /*
    * init the ralf_t struct with our lora radio device driver
    */
   smtc_modem_set_radio_context( radio_context );

   /*
    * check LR11XX Firmware version
    */
   if ( lr11xx_firmware_check( radio_context ) != LR11XX_HAL_STATUS_OK ) {
      printk( "Something goes wrong with the LR11XX firmware, stay in BLE mode and update it\n" );
#if 0
      tracker_app_init_context( dev_eui, join_eui, app_key );

      while( 1 ) {
	 // Stay in BLE while the LR11XX firmware is not installed
	 start_ble_thread( 0, stack_id );
      }
#endif
   }

#if 0
    /* Init board and peripherals */
    hal_mcu_init( );
    smtc_board_init_periph( );
#endif

    {
       static apps_modem_event_callback_t apps_modem_event_callback = {
	  .down_data                 = on_down_data,
	  .joined                    = on_joined,
	  .reset                     = on_reset,
	  .gnss_scan_done            = on_gnss_scan_done,
	  .gnss_terminated           = on_gnss_scan_terminated,
	  .wifi_scan_done            = on_wifi_scan_done,
	  .wifi_terminated           = on_wifi_scan_terminated,
//	  .alarm                     = on_alarm_event,
	  .lorawan_mac_time          = on_lorawan_mac_time,
	  .link_status               = on_link_status,
//	  .gnss_almanac_demod_update = on_gnss_almanac_demod_update,
       };

       /* Init the Lora Basics Modem event callbacks */
       apps_modem_event_init( &apps_modem_event_callback );
    }

    /*
     * setup modem callbacks
     */
    smtc_modem_init( &apps_modem_event_process );

    /*
     * main application startup
     */
    app_start( );

    while( 1 ) {

	/*
	 * run the modem engine.  The return value is the number of mSecs until we need to run again.
	 */
	sleep_msecs = smtc_modem_run_engine( );

	/*
	 * Wait for the specified time, or for a command to come in.  The engine did return
	 * a value of 0, which implies wait forevcer to k_msgq_get(), so protect from that.
	 */
	if (sleep_msecs) {
	   rc = k_msgq_get(&semtracker_msgq, &msg, K_MSEC( sleep_msecs ));
	   if (rc == 0) {
	      semtracker_msg_handle(&msg);
	   }
	}

#if 0


	if( get_hall_effect_irq_state( ) == true )
	{
	    /* Effect hall irq, reset the board*/
	    smtc_modem_hal_reset_mcu( );
	}
	else
	{
	    /* go in low power */
	    hal_mcu_disable_irq( );
	    if( smtc_modem_is_irq_flag_pending( ) == false )
	    {
		hal_mcu_set_sleep_for_ms( sleep_time_ms );
	    }
	    hal_mcu_enable_irq( );

	    if( tracker_ctx.airplane_mode == false )
	    {
		/* Wake up from static mode thanks the accelerometer ? */
		if( ( get_accelerometer_irq1_state( ) == true ) &&
		    ( tracker_app_is_tracker_in_static_mode( ) == true ) )
		{
		    /* Start Hall Effect sensors while the tracker moves */
		    smtc_board_hall_effect_enable( true );

		    /* abort and relauch middlewares */
		    HAL_DBG_TRACE_INFO( "WAKE UP ABORT CURRENT RP TASKS\n" );
		    if( smtc_modem_gnss_scan_cancel( stack_id ) == SMTC_MODEM_RC_OK )
		    {
			smtc_modem_gnss_scan_aggregate( stack_id, false );
			smtc_modem_gnss_scan( stack_id, SMTC_MODEM_GNSS_MODE_MOBILE, 0 );
		    }
		    smtc_modem_wifi_scan_cancel( stack_id );
		}
	    }
	    else
	    {
		/* Wake up thanks the accelerometer and in airplane mode ? */
		if( is_accelerometer_detected_moved( ) == true )
		{
		    smtc_board_hall_effect_enable_for_duration( TRACKER_AIRPLANE_HALL_EFFECT_TIMEOUT_MS );
		    HAL_DBG_TRACE_PRINTF( "Start hall effect sensor for %ds\n",
					  TRACKER_AIRPLANE_HALL_EFFECT_TIMEOUT_MS / 1000 );
		}
	    }
	}
#endif
    } // while(1)
}


/*-------------------------------------------------------------------------
 *
 * name:        semtracker_msg_handle
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void semtracker_msg_handle (semtracker_msg_t *msg)
{
   switch(msg->cmd) {
      /*
       * This command says we should perform a GNSS request.
       */
       case SEMTRACKER_CMD_GNSS_SCAN:

	  printf("%s %d \n", __func__,__LINE__);

	  smtc_modem_gnss_scan_aggregate( stack_id, true );
	  smtc_modem_gnss_scan( stack_id, SMTC_MODEM_GNSS_MODE_STATIC, 5 );
	  break;

	  /*
	   * This command says we should perform a WiFi scan
	   */
       case SEMTRACKER_CMD_WIFI_SCAN:

	  printf("%s %d \n", __func__,__LINE__);

	  smtc_modem_wifi_scan( stack_id, 0);
	  break;

      /*
       * This command is just to wake the thread.  NOP other than that.
       */
       case SEMTRACKER_CMD_WAKEUP:
	  break;

       default:
	  printf("%s: Unhandled command: %d \n", __func__, msg->cmd);
   }
}

/*-------------------------------------------------------------------------
 *
 * name:        display_lbm_version
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void display_lbm_version( void )
{
   smtc_modem_return_code_t
      rc;
   smtc_modem_version_t
      modem_version;

   rc = smtc_modem_get_modem_version( &modem_version );

   if ( rc == SMTC_MODEM_RC_OK ) {
      printk( "LoRa Basics Modem version: %.2x.%.2x.%.2x\n",
	      modem_version.major,
	      modem_version.minor,
	      modem_version.patch );
   }
}

/*-------------------------------------------------------------------------
 *
 * name:        init_context
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void init_config (void )
{
   /*
    * Attempt to restore configuration from settings
    */
   if ( config_restore( &config ) ) {
	 printk("%s %d (from %p) \n", __func__,__LINE__, __builtin_return_address(0) );

   #if 0
      /* When the production keys are used, DevEUI = ChipEUI and JoinEUI is the one defined in
       * lorawan_key_config.h
       */
      if( tracker_ctx.has_lr11xx_trx_firmware )
      {
	 smtc_modem_get_chip_eui( stack_id, dev_eui );
      }
#endif
      /*
       * error reading context, so use programattic values.
       */
      config_defaults ( &config );

#if 0
      /* Init the tracker internal log context */
      if( tracker_init_internal_log_ctx( ) != TRACKER_SUCCESS )
      {
	    HAL_DBG_TRACE_ERROR( "tracker_init_internal_log_ctx failed\n" );
      }
#endif
   }
   else {
	 printk("%s %d (from %p) \n", __func__,__LINE__, __builtin_return_address(0) );

#if 0
       /* Restore the tracker internal log context */
       if( tracker_restore_internal_log_ctx( ) != TRACKER_SUCCESS )
       {
	  /* Init the tracker internal log context */
	  if( tracker_init_internal_log_ctx( ) != TRACKER_SUCCESS )
	  {
	     HAL_DBG_TRACE_ERROR( "tracker_init_internal_log_ctx failed\n" );
	  }
       }

       /* Set the restored LoRaWAN Keys */
       memcpy( dev_eui, tracker_ctx.dev_eui, LORAWAN_DEVICE_EUI_LEN );
       memcpy( join_eui, tracker_ctx.join_eui, LORAWAN_JOIN_EUI_LEN );
       memcpy( app_key, tracker_ctx.app_key, LORAWAN_APP_KEY_LEN );
#endif
    }

#if 0
   /* Init tracker context volatile parameters */
   tracker_ctx.accelerometer_move_history = 1;
   tracker_ctx.voltage                    = hal_mcu_get_vref_level( );
   tracker_ctx.gnss_scan_charge_nAh       = 0;
   tracker_ctx.wifi_scan_charge_nAh       = 0;
   tracker_ctx.reset_board_asked          = false;

   smtc_board_select_gnss_antenna( tracker_ctx.gnss_antenna_sel );

   /* Set the maximum authorized transmit output power supported by the board following the region */
   tracker_app_set_max_tx_power_supported( tracker_ctx.lorawan_region, tracker_ctx.lorawan_sub_region );

   /* Set the battery level */
   smtc_board_set_battery_level( tracker_get_battery_level( ) );
#endif
}

/*-------------------------------------------------------------------------
 *
 * name:        configure_lorawan_parms
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void configure_lorawan_parms( void )
{
    smtc_modem_return_code_t rc = SMTC_MODEM_RC_OK;

    printk( "LoRaWAN parameters:\n" );
#if 0
    rc = smtc_modem_get_chip_eui( stack_id, tracker_ctx.chip_eui );

    if ( rc == SMTC_MODEM_RC_OK ) {
       HAL_DBG_TRACE_ARRAY( "ChipEIU", tracker_ctx.chip_eui, SMTC_MODEM_EUI_LENGTH );
    }
    else
    {
       HAL_DBG_TRACE_ERROR( "smtc_modem_get_chip_eui failed (%d)\n", rc );
    }
#endif



    rc = smtc_modem_set_deveui( stack_id, config.dev_eui );

    if( rc == SMTC_MODEM_RC_OK ) {
       HAL_DBG_TRACE_ARRAY( "DevEUI ", config.dev_eui, SMTC_MODEM_EUI_LENGTH );
    }
    else {
       printk( "smtc_modem_get_deveui failed (%d)\n", rc );
    }




    rc = smtc_modem_set_joineui( stack_id, config.join_eui );

    if( rc == SMTC_MODEM_RC_OK ) {
       HAL_DBG_TRACE_ARRAY( "JoinEUI", config.join_eui, SMTC_MODEM_EUI_LENGTH );
    }
    else {
       printk( "smtc_modem_get_joineui failed (%d)\n", rc );
    }


#if 0
    /* The Derive keys is done thought the smtc_modem_get_pin command */
    rc = smtc_modem_get_pin( stack_id, tracker_ctx.lorawan_pin );
    if( rc == SMTC_MODEM_RC_OK )
    {
	HAL_DBG_TRACE_ARRAY( "PIN", tracker_ctx.lorawan_pin, 4 );
    }
    else
    {
	HAL_DBG_TRACE_ERROR( "smtc_modem_get_pin failed (%d)\n", rc );
    }
#endif

//    if( tracker_ctx.use_semtech_join_server == false )
    {
       rc = smtc_modem_set_nwkkey( stack_id, config.app_key );
       if( rc == SMTC_MODEM_RC_OK ) {
	  HAL_DBG_TRACE_ARRAY( "AppKey", config.app_key, SMTC_MODEM_KEY_LENGTH );
       }
       else {
	  HAL_DBG_TRACE_ERROR( "smtc_modem_set_nwkkey failed (%d)\n", rc );
       }
    }
//    else {
//       printk( "AppKey : Use Semtech Join Sever\n" );
//    }

    smtc_modem_set_region( stack_id, config.region );
    printk("Region: %s\n", smtc_modem_region_to_str( config.region ));
}



/*-------------------------------------------------------------------------
 *
 * name:        set_data_rate_profile
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void set_data_rate_profile( uint32_t region )
{
   uint8_t
      profile[SMTC_MODEM_CUSTOM_ADR_DATA_LENGTH],
      rate;
   uint32_t
      i;

   /*
    * Use a rate of 1 for the US915.  3 for all others.
    */
   rate = (region ==  SMTC_MODEM_REGION_US_915 ? 1 : 3);

   for (i=0; i < ARRAY_SIZE(profile); i++) {
      profile[i] = rate;
   }

   smtc_modem_adr_set_profile( stack_id, SMTC_MODEM_ADR_PROFILE_CUSTOM, profile );
   smtc_modem_set_nb_trans( stack_id, 3 );
}


#if 0

static void tracker_app_build_and_send_tracker_settings( const uint8_t* buffer, uint8_t len )
{
    uint8_t tx_max_payload = 0;

    ASSERT_SMTC_MODEM_RC( smtc_modem_get_next_tx_max_payload( stack_id, &tx_max_payload ) );
    HAL_DBG_TRACE_PRINTF( "tx_max_payload %d \n", tx_max_payload );

    HAL_DBG_TRACE_PRINTF( " - Tracker settings (%d bytes) : ", len + 2 );

    if( tx_max_payload < ( len + 2 ) )
    {
	HAL_DBG_TRACE_ERROR( "TX max payload < payload len\n" );
    }
    else
    {
	uint8_t payload_len = 0;
	uint8_t lorawan_payload[242];

	/* Add tracker settings value */
	lorawan_payload[payload_len++] = TLV_TRACKER_SETTINGS_TAG;  // Tracker settings TAG
	lorawan_payload[payload_len++] = len;                       // Tracker settings LEN is variable

	memcpy( lorawan_payload + payload_len, buffer, len );
	payload_len += len;

	HAL_DBG_TRACE_MSG( "Send data\n" );
	smtc_modem_request_uplink( stack_id, TRACKER_REQUEST_MSG_PORT, false, lorawan_payload, payload_len );
    }
}

static void tracker_app_read_and_send_sensors( void )
{
    uint8_t  tx_max_payload = 0;
    uint8_t  payload_len    = 0;
    uint8_t  lorawan_payload[242];
    uint32_t charge_mah = 0;

    /* SENSORS DATA */
    HAL_DBG_TRACE_INFO( "*** sensors collect ***\n" );

    /* Move history */
    HAL_DBG_TRACE_PRINTF( "Move history : %d\n", tracker_ctx.accelerometer_move_history );

    /* Temperature */
    tracker_ctx.temperature = smtc_modem_hal_get_temperature( );
    HAL_DBG_TRACE_PRINTF( "Temperature : %d *C\n", tracker_ctx.temperature );

    /* Modem charge */
    smtc_modem_get_charge( &charge_mah );
    HAL_DBG_TRACE_PRINTF( "LBM Charge value : %d mAh\n", charge_mah );
    HAL_DBG_TRACE_PRINTF( "GNSS scan charge value : %d nAh\n", tracker_ctx.gnss_scan_charge_nAh );
    HAL_DBG_TRACE_PRINTF( "Wi-Fi scan charge value : %d nAh\n", tracker_ctx.wifi_scan_charge_nAh );
    tracker_app_store_new_accumulated_charge( charge_mah + ( tracker_ctx.gnss_scan_charge_nAh / 1000000 ) +
					      ( tracker_ctx.gnss_scan_charge_nAh / 1000000 ) );
    HAL_DBG_TRACE_PRINTF( "Accumulated charge value : %d mAh\n", tracker_ctx.accumulated_charge_mAh );

    /* Board voltage charge */
    tracker_ctx.voltage = smtc_modem_hal_get_voltage_mv( ) * 20;
    HAL_DBG_TRACE_PRINTF( "Board voltage : %d mV\n\n", tracker_ctx.voltage );

    ASSERT_SMTC_MODEM_RC( smtc_modem_get_next_tx_max_payload( stack_id, &tx_max_payload ) );
    HAL_DBG_TRACE_PRINTF( "tx_max_payload %d \n", tx_max_payload );

    /* Temperature */
    lorawan_payload[payload_len++] = tracker_ctx.temperature >> 8;
    lorawan_payload[payload_len++] = tracker_ctx.temperature;

    /* Accumulated Charge in mAh */
    lorawan_payload[payload_len++] = tracker_ctx.accumulated_charge_mAh >> 8;
    lorawan_payload[payload_len++] = tracker_ctx.accumulated_charge_mAh;

    /* Board Voltage */
    lorawan_payload[payload_len++] = tracker_ctx.voltage >> 8;
    lorawan_payload[payload_len++] = tracker_ctx.voltage;

    if( tx_max_payload < payload_len )
    {
	HAL_DBG_TRACE_ERROR( "TX max payload < payload len\n" );
    }
    else
    {
	HAL_DBG_TRACE_MSG( "Use classic uplink to send data\n" );
	smtc_modem_request_uplink( stack_id, TRACKER_APP_SENSOR_PORT, false, lorawan_payload, payload_len );
    }
}

static void tracker_app_store_new_accumulated_charge( uint32_t charge_mAh )
{
    static uint32_t previous_charge_mAh = 0;  // Previous modem charge before read from LBM plus scans, keep
					      // the historic even after leave the function because of the static

    /* Store the new accumulated charge only if the modem charge has changed */
    if( charge_mAh != previous_charge_mAh )
    {
	tracker_ctx.accumulated_charge_mAh += charge_mAh - previous_charge_mAh;
	HAL_DBG_TRACE_MSG( "New accumulated charge stored\n" );
	tracker_store_app_ctx( );

	previous_charge_mAh = charge_mAh;

	/* Set the new battery level */
	smtc_board_set_battery_level( tracker_get_battery_level( ) );
    }
}

static bool tracker_app_is_tracker_in_static_mode( void )
{
    if( ( ( tracker_ctx.accelerometer_move_history & TRACKER_SEND_ONE_MORE_SCANS_ONCE_STATIC ) == 0 ) &&
	( tracker_ctx.accelerometer_used == 1 ) )
    {
	return true;
    }
    else
    {
	return false;
    }
}

static void tracker_app_parse_downlink_frame( uint8_t port, const uint8_t* payload, uint8_t size )
{
    switch( port )
    {
    case TRACKER_REQUEST_MSG_PORT:
    {
	uint8_t tag           = 0;
	uint8_t len           = 0;
	uint8_t payload_index = 0;

	while( payload_index < size )
	{
	    tag = payload[payload_index++];
	    len = payload[payload_index++];

	    switch( tag )
	    {
	    case GET_APP_TRACKER_SETTINGS_CMD:
	    {
		uint8_t settings_buffer[240];
		uint8_t tracker_settings_payload_max_len = 0;
		memcpy( settings_buffer, payload + payload_index, len );

		ASSERT_SMTC_MODEM_RC(
		    smtc_modem_get_next_tx_max_payload( stack_id, &tracker_settings_payload_max_len ) );

		HAL_DBG_TRACE_INFO( "###### ===== TRACKER CONFIGURATION SETTINGS PAYLOAD RECEIVED ==== ######\n\n" );

		tracker_ctx.tracker_settings_payload_len =
		    tracker_parse_cmd( stack_id, settings_buffer, tracker_ctx.tracker_settings_payload,
				       tracker_settings_payload_max_len, false );

		/* Store the new values here if it's asked */
		if( ( tracker_ctx.new_value_to_set ) == true )
		{
		    tracker_ctx.new_value_to_set = false;
		    tracker_store_app_ctx( );
		}

		if( ( tracker_ctx.lorawan_parameters_have_changed == true ) ||
		    ( tracker_ctx.reset_board_asked == true ) )
		{
		    /* reset device because of LoRaWAN Parameters */
		    HAL_DBG_TRACE_INFO( "###### ===== RESET TRACKER ==== ######\n\n" );
		    smtc_modem_hal_reset_mcu( );
		}

		tracker_app_build_and_send_tracker_settings( tracker_ctx.tracker_settings_payload,
							     tracker_ctx.tracker_settings_payload_len );

		break;
	    }
	    case GET_MODEM_DATE_CMD:
	    {
		uint8_t        buffer[6];
		const uint32_t modem_date = apps_modem_common_get_utc_time( stack_id );

		buffer[0] = GET_MODEM_DATE_CMD;
		buffer[1] = GET_MODEM_DATE_ANSWER_LEN;
		buffer[2] = modem_date >> 24;
		buffer[3] = modem_date >> 16;
		buffer[4] = modem_date >> 8;
		buffer[5] = modem_date;

		/* Use the emergency TX to reduce the latency */
		smtc_modem_request_emergency_uplink( stack_id, port, false, buffer, 6 );

		HAL_DBG_TRACE_INFO( "###### ===== SEND LBM DATE IN EMERGENCY TX ==== ######\n\n" );

		break;
	    }
	    default:
		payload_index += len;
		break;
	    }
	}
    }
    default:
	break;
    }
}

static void tracker_app_set_max_tx_power_supported( smtc_modem_region_t region, smtc_modem_sub_region_t sub_region )
{
    int8_t max_tx_power_supported = 22;

    switch( region )
    {
    case SMTC_MODEM_REGION_IN_865:
    case SMTC_MODEM_REGION_AU_915:
	max_tx_power_supported = 14;
	break;
    case SMTC_MODEM_REGION_AS_923_GRP1:
	if( sub_region == SMTC_MODEM_SUB_REGION_JAPAN )
	{
	    max_tx_power_supported = 9;
	}
	break;
    case SMTC_MODEM_REGION_KR_920:
	max_tx_power_supported = 8;
	break;
    default:
	break;
    }

    smtc_board_set_max_tx_power_supported( max_tx_power_supported );
}

/*
 * -----------------------------------------------------------------------------
 * --- TRACKER LORA BASICS MODEM EVENT FUNCTION TYPES --------------------------
 */

static void on_alarm_event( void )
{
    smtc_modem_alarm_start_timer( APP_ONE_DAY_IN_SEC );

    /* Trig Mac command */
    ASSERT_SMTC_MODEM_RC( smtc_modem_trig_lorawan_mac_request(
	stack_id, SMTC_MODEM_LORAWAN_MAC_REQ_LINK_CHECK | SMTC_MODEM_LORAWAN_MAC_REQ_DEVICE_TIME ) );
}

static void on_gnss_almanac_demod_update( smtc_modem_almanac_demodulation_event_data_almanac_update_t data )
{
    if( ( data.status_gps == SMTC_MODEM_GNSS_ALMANAC_UPDATE_STATUS_COMPLETED ) &&
	( data.status_beidou == SMTC_MODEM_GNSS_ALMANAC_UPDATE_STATUS_COMPLETED ) )
    {
	HAL_DBG_TRACE_INFO( "Almanac update completed\n" );
    }
}

#endif

#define SEMTRACKER_STACKSIZE (2048)

K_THREAD_STACK_DEFINE(semtracker_stack_area, SEMTRACKER_STACKSIZE);
struct k_thread	semtracker_kthread;

/*-------------------------------------------------------------------------
 *
 * name:        semtracker_init
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
void semtracker_init(void)
{
   k_tid_t tid;

   tid = k_thread_create(&semtracker_kthread, semtracker_stack_area,
			 K_THREAD_STACK_SIZEOF(semtracker_stack_area),
			 semtracker_thread,
			 NULL, NULL,	NULL,
			 PRIORITY_SEMTRACKER, 0, K_NO_WAIT);

   k_thread_name_set(tid, "semtracker");

}

/*-------------------------------------------------------------------------
 *
 * name:        semtracker_cmd
 *
 * description: Send a command to the semtracker thread via its message Q
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
void semtracker_cmd (semtracker_cmd_e cmd, uint32_t arg0)
{
   semtracker_msg_t
      msg;

   msg.cmd  = cmd;
   msg.arg0 = arg0;

   k_msgq_put(&semtracker_msgq, &msg, K_FOREVER);
}

/*-------------------------------------------------------------------------
 *
 * name:        semtracker_thread_wakeup
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
void semtracker_thread_wakeup (void)
{
   semtracker_cmd(SEMTRACKER_CMD_WAKEUP, 0);
}
