#include <stdio.h>
#include <string.h>
#include "join.h"
#include "misc.h"

static const char *delim = "/";

void join_parse (char *topic_name)
{
   char
      *cp = strtok(topic_name, delim);

   /*
    * This token seems to be some kind of application ID
    */
   cp  = strtok(NULL, delim);

   /*
    * This token is "device"
    */
   cp  = strtok(NULL, delim);

   /*
    * This is the ID of the device.
    */
   cp  = strtok(NULL, delim);

   print_cur_time();

   printf("Device: '%s' Joined network\n", cp);
}
