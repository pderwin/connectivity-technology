#include <stdarg.h>
#include <sys/timeb.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include "MQTTClient.h"
#include "camera.h"
#include "home_security.h"
#include "join.h"
#include "up.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

static void process_message (char *topic_name, MQTTClient_message *m);

static options_t options =
{
   "tcp://localhost:1883",
   NULL,
   "tcp://localhost:1884",
   0,
   0,
   0,
   MQTTVERSION_DEFAULT,
   1,
};


/*-------------------------------------------------------------------------
 *
 * name:        mqtt_init
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
int mqtt_init(uint32_t timeout_secs)
{
   int
      rc = 0,
      subsqos = 0;
   char
      *all_topics = "#",
      *topic_name  = NULL;
   int
      topicLen;
   MQTTClient
      c;
   MQTTClient_connectOptions
      opts = MQTTClient_connectOptions_initializer;
   MQTTClient_message
      *m = NULL;
   uint32_t
      seconds;


   rc = MQTTClient_create(
      &c,                          // handle
      options.connection,          // server URI
      "monitor_pir",               // client ID
      MQTTCLIENT_PERSISTENCE_NONE, // messages are lost if connection lost
      NULL                         // NULL since PERSISTENCE_NONE
      );

   /*
    * If client create is not successful, then abort.
    */
   if (rc != MQTTCLIENT_SUCCESS) {
      printf("%s: Error during client create\n", __func__);
      goto exit;
   }

   opts.cleansession = 1;
   opts.MQTTVersion = options.MQTTVersion;

   /*
    * Set keep alive interval to 10 minutes
    */
   opts.keepAliveInterval = 10;

   rc = MQTTClient_connect(c, &opts);

   if (rc != MQTTCLIENT_SUCCESS) {
      printf("%s: Error connecting to server\n", __func__);
      goto exit;
   }

   /*
    * Subscribe to topic
    */
   rc = MQTTClient_subscribe(c, all_topics, subsqos);

   if (rc != MQTTCLIENT_SUCCESS) {
      printf("%s: Error subscribing to all topics\n", __func__);
      exit(1);
   }

   /*
    * Exit every 24 hours, just to pick up new executable in case something changed.
    */
   for (seconds = 0; seconds < timeout_secs; seconds++) {

      /*
       * Wait for message for 5 minutes.  Timeout value is in terms of mSecs.
       */
      rc = MQTTClient_receive(c, &topic_name, &topicLen, &m, 5000);

      if (rc != MQTTCLIENT_SUCCESS) {
	 printf("%s: timeout waiting for event recipt (%d) \n", __func__, seconds);
	 break;
      }

      if (m) {

	 process_message(topic_name, m);

	 MQTTClient_freeMessage(&m);
	 MQTTClient_free(topic_name);
      }


#define TWIDDLE_SIZE ARRAY_SIZE(twiddle)

      {
	 static const char *twiddle[] = {
	    "|",
	    "/",
	    "-",
	    "\\"
	 };

	 printf("%s %5d\r", twiddle[seconds % TWIDDLE_SIZE], timeout_secs - seconds);
	 fflush(stdout);
      }
   }





   rc = MQTTClient_unsubscribe(c, all_topics);

   if (rc != MQTTCLIENT_SUCCESS) {
      printf("%s: error during Unsubscribe.  rc: %d \n", __func__, rc);
      exit(1);
   }


   rc = MQTTClient_disconnect(c, 0);
   if (rc != MQTTCLIENT_SUCCESS) {
      printf("%s: error during Disconnect.  rc: %d \n", __func__, rc);
      exit(1);
   }

   MQTTClient_destroy(&c);

  exit:

   return 0;
}


/*-------------------------------------------------------------------------
 *
 * name:        process_message
 *
 * description: Check the topic name and see if this is a message that we
 *              want to parse.  If so, then call parsing routine.
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void process_message (char *topic_name, MQTTClient_message *m)
{
   uint32_t len = strlen(topic_name);

   /*
    * only want 'application' topics
    */
   if (memcmp(topic_name, "application/", 12)) {
      return;
   }

   /*
    * Process 'join' packets.
    */
   if (! strcmp(topic_name + len - 5, "/join")) {
      join_parse(topic_name);
      return;
   }

   /*
    * Only want to process 'up' packets
    */
   if (!strcmp(topic_name + len - 3, "/up")) {
      up_parse(m->payload, m->payloadlen);
      return;
   }
}
