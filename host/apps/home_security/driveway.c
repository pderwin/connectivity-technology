#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

void driveway_alert (void)
{
   char
      *am_pm = "AM",
      cmd[128],
      *tmp_filename = "/tmp/driveway.alert",
      *weekdays[] = { "Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
   uint32_t
      tm_hour;
   FILE
      *fp;
   struct tm
      *tm;

   {
      time_t tim;

      time(&tim);
      tm = localtime( &tim );
   }

   fp = fopen(tmp_filename, "w");
   fprintf(fp, "Driveway alert\n");
   fclose(fp);

   tm_hour = tm->tm_hour;

   if (tm_hour > 12) {
      tm_hour -= 12;

      am_pm = "PM";
   }


   sprintf(cmd, "mail -s \"Driveway Alert: %02d:%02d %s - %s %02d/%02d/%02d \" lkerwin2003@yahoo.com < %s",
	   tm_hour,
	   tm->tm_min,
	   am_pm,
	   weekdays[tm->tm_wday],
	   tm->tm_mon + 1,
	   tm->tm_mday,
	   tm->tm_year + 1900,
	   tmp_filename);

   printf("%s\n", cmd);
   system (cmd);
}
