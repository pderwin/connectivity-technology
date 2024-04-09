#include "config.h"

static uint8_t
    default_app_key [ LORAWAN_APP_KEY_LEN  ] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe },
    default_dev_eui [ SMTC_MODEM_EUI_LENGTH] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b, 0x03 },
    default_join_eui[ SMTC_MODEM_EUI_LENGTH] = { 0x70, 0xB3, 0xD5, 0x7E, 0xD0, 0x03, 0x31, 0xC9 };

/*-------------------------------------------------------------------------
 *
 * name:        config_defaults
 *
 * description: Apply some default values to the configuration.  Called
 *              when the restore fails.
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
void config_defaults (config_t *config)
{
   memcpy( config->dev_eui,  default_dev_eui,  SMTC_MODEM_EUI_LENGTH  );
   memcpy( config->join_eui, default_join_eui, SMTC_MODEM_EUI_LENGTH );
   memcpy( config->app_key,  default_app_key,  LORAWAN_APP_KEY_LEN );

   config->region = SMTC_MODEM_REGION_US_915;
}


/*-------------------------------------------------------------------------
 *
 * name:        config_restore
 *
 * description:
 *
 * input:
 *
 * output:      0 - success
 *              1 - error
 *
 *-------------------------------------------------------------------------*/
uint32_t config_restore( config_t *config )
{
   return 1;
}
