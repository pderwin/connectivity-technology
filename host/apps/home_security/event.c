#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "base64.h"
#include "camera.h"
#include "event_internal.h"
#include "misc.h"

static event_t event;

void
    driveway_alert(void);

/*-------------------------------------------------------------------------
 *
 * name:        event_init
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
void event_init (void)
{
   bzero(&event,sizeof(event));;
}


/*-------------------------------------------------------------------------
 *
 * name:        event_key_value
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
void event_key_value (char *key, char *value)
{
   uint8_t
      *str;

   if (!strcmp(key, "fPort")) {
      event.f_port = atoi(value);
   }

   if (!strcmp(key, "data")) {

      if (strlen(value)) {
	 uint32_t
	    decode_size = 128;

	 str = base64_decode((uint8_t *) value, strlen(value), &decode_size);

	 memcpy(event.payload, str, decode_size);
	 free(str);

	 /*
	  * Add NULL termination to the string.
	  */
	 event.payload[decode_size] = 0;
      }
      else {
	 event.payload[0] = 0;
      }
   }

}

/*-------------------------------------------------------------------------
 *
 * name:        event_process
 *
 * description: Called at the end of the YAML parsing to process the content
 *              of the event.
 *
 * input:       none
 *
 * output:      none
 *
 *-------------------------------------------------------------------------*/
void event_process (void)
{

   /*
    * If no fport, then I don't know what to do with the packet.
    */
   if (!event.f_port) {
      return;
   }

   switch (event.f_port) {

       case 60:
	  print_cur_time();
	  printf("PIR: %s \n", event.payload);
	  break;

       case 70:
	  print_cur_time();
	  printf("DRIVEWAY_SENSOR: %s \n", event.payload);

	  driveway_alert();

	  /*
	   * Send request to the camera interaction thread to have it pull the
	   * correct footage from the camera to cover our time period
	   */
//	  camera_download(time(NULL));
	  break;

	  /*
	   * ignore certain ports for now
	   */
       case 111:
       case 199:
	  break;

       default:
	  printf("Unknown fport: %d \n", event.f_port);
	  break;
   }
}
