#include <zephyr.h>
#include "priority.h"

extern char *git_version;

/**
 ****************************************************************************************************
 *
 * \brief    Basic run-once initial setup of crypto anchor
 *
 * \param
 *
 * \returns
 *
 *****************************************************************************************************/
static void lora_entry (void *p1, void *p2, void *p3)
{
   unsigned int count = 0;

   printk("LoRa is alive (%s) \n", git_version);

   while(1) {
      k_sleep (K_MSEC(5000));

      printk("tick %d... \n", count++);
   }
}

#define LORA_STACKSIZE (0x400)

K_THREAD_DEFINE(lora_id,
		LORA_STACKSIZE,
		lora_entry,
		NULL, NULL, NULL,
		PRIORITY_LORA,
		0,
		0);

Z_INIT_LIBRARY(lora);
