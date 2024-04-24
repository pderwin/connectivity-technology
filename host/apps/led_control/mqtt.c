#include <stdarg.h>
#include <sys/timeb.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include "MQTTClient.h"
#include "mqtt.h"

static void mqtt_disconnect (void);

typedef struct {
   char* connection;         /**< connection to system under test. */
   char** haconnections;
   char* proxy_connection;
   int hacount;
   int verbose;
   int test_no;
   int MQTTVersion;
} options_t;

static options_t options =
{
   "tcp://localhost:1883",
   NULL,
   "tcp://localhost:1884",
   0,
   0,
   0,
   MQTTVERSION_DEFAULT,
};

static MQTTClient c;
static MQTTClient_deliveryToken deliveredtoken;

void connlost(void *context, char *cause)
{
   (void) context;

   printf("\nConnection lost\n");
   printf("     cause: %s\n", cause);
}

void delivered(void *context, MQTTClient_deliveryToken dt)
{
   (void) context;

   printf("Message with token value %d delivery confirmed\n", dt);
   deliveredtoken = dt;
}

int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
   (void) context;
   (void) topicName;
   (void) topicLen;

   printf("Message arrived\n");
   printf("     topic: %s\n", topicName);
   printf("   message: %.*s\n", message->payloadlen, (char*)message->payload);
   MQTTClient_freeMessage(&message);
   MQTTClient_free(topicName);
   return 1;
}

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
void mqtt_init(void)
{
   int
      rc = 0;
   MQTTClient_connectOptions
      opts = MQTTClient_connectOptions_initializer;


   rc = MQTTClient_create(
      &c,                          // handle
      options.connection,          // server URI
      "",               // client ID
      MQTTCLIENT_PERSISTENCE_NONE, // messages are lost if connection lost
      NULL                         // NULL since PERSISTENCE_NONE
      );

   /*
    * If client create is not successful, then abort.
    */
   if (rc != MQTTCLIENT_SUCCESS) {
      printf("%s: Error during client create\n", __func__);
      exit(1);
   }

#if 0
   if ((rc = MQTTClient_setCallbacks(&c, NULL, connlost, msgarrvd, delivered)) != MQTTCLIENT_SUCCESS)
   {
      printf("Failed to set callbacks, return code %d\n", rc);
      rc = EXIT_FAILURE;
      exit(1);
   }
#endif

   opts.cleansession = 1;
   opts.MQTTVersion = options.MQTTVersion;

   opts.username = "admin";
   opts.password = "admin";

   /*
    * Set keep alive interval to 60 minutes
    */
   opts.keepAliveInterval = 60;

   rc = MQTTClient_connect(c, &opts);

   if (rc != MQTTCLIENT_SUCCESS) {
      printf("%s: Error connecting to server\n", __func__);
      exit(1);
   }

   atexit( mqtt_disconnect );
}

/*-------------------------------------------------------------------------
 *
 * name:        mqtt_disconnect
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void mqtt_disconnect (void)
{
   int
      rc = 0;

   printf("%s %d (from %p) \n", __func__,__LINE__, __builtin_return_address(0) );

   rc = MQTTClient_disconnect(c, 0);

   if (rc != MQTTCLIENT_SUCCESS) {
      printf("%s: error during Disconnect.  rc: %d \n", __func__, rc);
      exit(1);
   }

   MQTTClient_destroy(&c);
}

#define TIMEOUT     10000L

void mqtt_publish (char *topic_name, char *payload, uint32_t payload_len)
{
   int
      rc;
   MQTTClient_message
      pubmsg = MQTTClient_message_initializer;
   MQTTClient_deliveryToken
      token;

#if 0
   {
      uint32_t
	 i;
      printf("topic: '%s' \n", topic_name);
      printf("%s %d len: %d \n---\n", __func__,__LINE__, payload_len);

      for (i=0; i<payload_len; i++) {
	 printf("%c", payload[i]);
      }
      printf("\n---\n");
   }
#endif


    pubmsg.payload    = payload;
    pubmsg.payloadlen = payload_len;

    pubmsg.qos = 0;
    pubmsg.retained = 0;

    deliveredtoken = 0;

   rc = MQTTClient_publishMessage(c, topic_name, &pubmsg, &token );

   if (rc !=  MQTTCLIENT_SUCCESS) {
      printf("Failed to publish message\n");
      exit(1);
   }

//   printf("Waiting for publication ...\n");
//   rc = MQTTClient_waitForCompletion(c, token, TIMEOUT);

}
