#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "base64.h"
#include "event_internal.h"

static event_t event;
static void print_cur_time (void);

void event_init (void)
{
   bzero(&event,sizeof(event));;
}


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

void event_process (void)
{

   /*
    * If no fport, then I don't know what to do with the packet.
    */
   if (!event.f_port) {
      return;
   }

   print_cur_time();

   switch (event.f_port) {
       case 60:
	  printf("PIR: %s", event.payload);
	  break;
       case 70:
	  printf("DRIVEWAY_SENSOR: %s", event.payload);
	  break;
       default:
	  printf("Unknown fport: %d ", event.f_port);
	  break;
   }

   printf("\n");
}

static void print_cur_time (void)
{
   char
      time_buf[80];
   struct timeval
      ts;
   struct tm
      timeinfo;

   gettimeofday(&ts, NULL);
   localtime_r(&ts.tv_sec, &timeinfo);

   strftime(time_buf, sizeof(time_buf), "%m/%d/%Y %H:%M:%S", &timeinfo);

   printf("%s: ", time_buf);
}
