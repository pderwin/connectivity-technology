#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include "misc.h"

/*-------------------------------------------------------------------------
 *
 * name:
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
void print_cur_time (void)
{
   char
      time_buf[80];
   struct timeval
      ts;
   struct tm
      timeinfo;

   gettimeofday(&ts, NULL);
   localtime_r(&ts.tv_sec, &timeinfo);

   strftime(time_buf, sizeof(time_buf), "%m/%d/%Y %H:%M:%S", &timeinfo);

   printf("%s: ", time_buf);
}
