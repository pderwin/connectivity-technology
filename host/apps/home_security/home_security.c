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
// #include "MQTTClient.h"
#include "camera.h"
#include "home_security.h"
// #include "message.h"
#include "mqtt.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

int main(int argc, char *argv[])
{
   char
      *camera_download_path = ".";
   uint32_t
      camera_download_range = 0,
      mqtt_timeout = (24 * 60 * 60);  /* default to timeout after 24 hours */
   time_t
      camera_download_time = 0;

   setbuf(stdout, NULL);

   printf("Home_security monitor V0.1...\n");

   {
      int c;

      static struct option long_options[] = {
	 {"camera-download",       required_argument, 0, 'c'},
	 {"camera-download-path",  required_argument, 0, 'p'},
	 {"camera-download-range", required_argument, 0, 'r'},
	 {"camera-ip",             required_argument, 0, 'i'},
	 {"help",                  0,                 0,   0},
	 {"timeout",               required_argument, 0, 't'},
	 {0,                       0,                 0,   0}
      };

      while (1) {
	 int option_index = 0;

	 c = getopt_long (argc, argv, "e:hntv:", long_options, &option_index);
	 if (c == -1)
	    break;

	 switch (c) {

	     case 'c': /* camera download */
		camera_download_time = strtoull(optarg, NULL, 0);
		break;

	     case 'i': /* camera IP address */
		camera_ip_address(optarg);
		break;

	     case 'p': /* camera download range */
		camera_download_path = strdup(optarg);
		break;

	     case 'r': /* camera download range */
		camera_download_range = strtoull(optarg, NULL, 0);
		break;

	     case 't': /* timeout */
		mqtt_timeout = atoi(optarg);
		printf("MQTT timeout %d secs\n", mqtt_timeout);
		break;

	     case 'x':
		printf("Read HEX formatted file\n");
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

   /*
    * Get thread created for interacting with the camera
    */
   camera_init(camera_download_time, camera_download_range, camera_download_path);

   /*
    * get thread created for interacting with MQTT port of chirpstack.
    */
   mqtt_init( mqtt_timeout );

   printf("Home Security exit\n");

   exit(0);
}
