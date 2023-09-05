#include <zephyr/kernel.h>
#include <zephyr/drivers/trace.h>

#include "radio_mutex.h"

static K_MUTEX_DEFINE(radio_mutex);


void radio_mutex_lock (void)
{
   TRACE(TAG_RADIO_MUTEX_LOCK);

   k_mutex_lock(&radio_mutex, K_FOREVER);
}

void radio_mutex_unlock (void)
{
   TRACE(TAG_RADIO_MUTEX_UNLOCK);

   k_mutex_unlock(&radio_mutex);
}
