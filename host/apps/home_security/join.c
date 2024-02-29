#include <stdio.h>
#include <string.h>
#include "join.h"

// 'application/3bdb1cb2-ed86-4b67-a0cd-e1113189aa3c/device/0000000000000b03/event/join'

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

   printf("Device: '%s' Joined network\n", cp);
}
