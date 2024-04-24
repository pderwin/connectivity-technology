#include <zephyr/kernel.h>
#include "downlink.h"
#include "alert.h"

void downlink_parse( uint8_t f_port, const uint8_t* payload, uint8_t length)
{
   uint32_t
      i;

   printk("%s %d f_port: %d \n", __func__,__LINE__, f_port );

   switch (f_port) {
       case 30:
	  printk("GOT LED CONTROL message.  Payload: '%s' l: %d \n", payload, length);

	  switch(payload[0]) {
	      case 0x55:
		 alert_set(1);
		 break;
	      case 0xaa:
		 alert_set(0);
		 break;
	      default:
		 printk("%s: unknown LED port command: %x\n", __func__, payload[0]);
		 break;
	  }

	  for (i=0; i<length; i++) {
	     printk("%02x ", payload[i]);
	  }
	  printk("\n");

	  break;

       default:
	  printk("%s: invalid port: %d \n", __func__, f_port);
	  break;
   }

}
