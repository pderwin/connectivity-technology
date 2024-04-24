#include <zephyr/kernel.h>
#include "lr1110_transceiver_0401.h"
#include "lr11xx_firmware_update.h"
#include "lr11xx_drv.h"    // zephyr driver interfaces
#include "lr11xx_hal.h"    // SMTC specified HAL layer
#include "lr11xx_system.h"

static lr11xx_fw_update_status_t firmware_update (const void *radio);

/*-------------------------------------------------------------------------
 *
 * name:        lr11xx_firmware_check
 *
 * description: Check that firmware is in transceiver mode, and has correct
 *              firmware version.  If not, cause the correct firmware to be
 *              loaded.
 *
 * input:       context - ptr to radio driver
 *
 * output:      LR11XX_HAL_STATUS_ERROR - error
 *              LR11XX_HAL_STATUS_OK    - success
 *
 *-------------------------------------------------------------------------*/
lr11xx_hal_status_t lr11xx_firmware_check ( const void *context )
{
   lr11xx_hal_status_t
      status;
   uint32_t
      has_expected_fw_version = false;
   uint8_t
      lr11xx_busy_pin_state   = 0;
   lr11xx_system_version_t
      fw_version;

    /*
     * Reset the chip
     */
    lr11xx_hal_reset( context );

    /*
     * Allow time for the chip to perform initialization.  Current LR1110 transceiver code takes
     * 200 mSec to drop busy
     */
    k_msleep(250);

    /*
     * check if the LR1110 is in transceiver mode or Modem-E mode, busy pin in low for transceiver mode, high for
     * Modem-E
     */
    lr11xx_busy_pin_state = lr11xx_drv_read_busy_pin( context );

    if ( lr11xx_busy_pin_state == 0 ) {

       /* LR1110 is in transceiver mode */
       status = lr11xx_system_get_version( context, &fw_version );

       if( ( status == LR11XX_HAL_STATUS_OK ) && ( fw_version.type == 0x0001 ) ) {

//	  tracker_ctx.has_lr11xx_trx_firmware = true;

	  if ( fw_version.fw < LR11XX_FIRMWARE_VERSION ) {

	     /* LR11XX firmware version is not the expected one.  Update the transceiver */
	     printk( "Wrong LR11XX_FW_VERSION, current version is 0x%04X, expected is 0x%04X\n",
		     fw_version.fw, LR11XX_FIRMWARE_VERSION );

	     printk("%s %d updating firmware \n", __func__,__LINE__);

	     firmware_update( context );

	     printk("%s %d done updating firmware \n", __func__,__LINE__);

	  }
	  else {
	     return LR11XX_HAL_STATUS_OK;
	  }
       }
       else {
	  printk( "NO FW\n" );
	  /* LR1110 has no firmware */
	  fw_version.fw = 0;  // Set the lr11xx_fw_version to 0 to be able to update it thought BLE
	  has_expected_fw_version             = false;
       }
    }
    else {
       printk( "Modem-E firmware\n" );
       fw_version.fw    = 0;  // Set the lr11xx_fw_version to 0 to be able to update it thought BLE
       has_expected_fw_version             = false;
    }

    return has_expected_fw_version;
}



static lr11xx_fw_update_status_t firmware_update (const void *radio)
{
   lr11xx_fw_update_status_t status;

   status = lr11xx_update_firmware( radio, LR11XX_FIRMWARE_UPDATE_TO, LR11XX_FIRMWARE_VERSION,
				    lr11xx_firmware_image, ( uint32_t ) LR11XX_FIRMWARE_IMAGE_SIZE );

   switch( status )
   {
       case LR11XX_FW_UPDATE_OK:
	  printk( "Expected firmware running!\n" );
	  printk( "Please flash another application (like EVK Demo App).\n" );
	  break;
       case LR11XX_FW_UPDATE_WRONG_CHIP_TYPE:
	  printk( "Wrong chip type!\n" );
	  break;

       case LR11XX_FW_UPDATE_ERROR:
	  printk( "Error! Wrong firmware version - please retry.\n" );
	  break;
   }

   return status;
}
