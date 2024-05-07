#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "base64.h"
#include "led.h"
#include "mqtt.h"

char payload[256];

static void payload_add(char *key, char *value, uint32_t add_comma)
{
   strcat(payload, "\"");
   strcat(payload, key);
   strcat(payload, "\": ");

   strcat(payload, value);

   if (add_comma) {
      strcat(payload, ", ");
   }
}

static void payload_append(char *str)
{
   strcat(payload, str);
}

void led_msg (char *address, uint32_t on_off)
{
   char
      *data_str,
      on_off_byte[1],
      *topic_name;
   uint32_t
      data_len;

   (void) address;

   payload_append("{");
   payload_add("devEui", "\"ff00000000000b03\"", 1);
   payload_add("confirmed", "true", 1);
   payload_add("fPort", "30", 1);

   if (on_off) {
      on_off_byte[0] = 0x55;
   }
   else {
      on_off_byte[0] = 0xaa;
   }

   data_str = base64_encode(on_off_byte, sizeof(on_off_byte), &data_len);

   sprintf(payload + strlen(payload), "\"data\": \"%*s\"}", data_len, data_str);

   topic_name = "application/3bdb1cb2-ed86-4b67-a0cd-e1113189aa3c/device/ff00000000000b03/command/down";

   mqtt_publish(topic_name, payload, strlen(payload));
}
