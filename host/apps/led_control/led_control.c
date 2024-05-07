#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <sys/timeb.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include "led.h"
#include "mqtt.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

int main(int argc, char *argv[])
{
   setbuf(stdout, NULL);

   printf("LED control V0.1...\n");

   /*
    * get thread created for interacting with MQTT port of chirpstack.
    */
   mqtt_init( );

   {
      int c;

      static struct option long_options[] = {
	 {"on",    required_argument, 0, 'n'},
	 {"off",   required_argument, 0, 'f'},
	 {"help",                  0, 0,   0},
	 {0,                       0, 0,   0}
      };

      while (1) {
	 int option_index = 0;

	 c = getopt_long (argc, argv, "f:hn:", long_options, &option_index);

	 if (c == -1)
	    break;

	 switch (c) {

	     case 'f': /* turn LED off*/
		led_msg(optarg, 0);
		break;

	     case 'n': /* turn LED no */
		led_msg(optarg, 1);
		break;

	     default:
	     case 'h':
		printf("parseTrace - parse Trace and likely create a listing\n\n");
		printf("Usage:\n");
		printf("\tparseSaleae input_file_name \n\n");
		printf("\tno-uart, n : don't print uart traffic\n");
		exit(0);
	 }
      }
   }

   exit(0);
}
