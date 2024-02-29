#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <sys/prctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "camera_internal.h"
#include "curl.h"
#include "hs_yaml.h"

static char
    *download_path,
    *ip_address;
static int
    socks[2];

static void     camera_thread           (time_t download_time, uint32_t download_range);
static void     check_for_video_timeout (time_range_t *time_range);
static void     display_time            (time_t download_time);
static uint32_t file_already_downloaded (const char *file_name, uint32_t file_size);

/*-------------------------------------------------------------------------
 *
 * name:        camera_download
 *
 * description: API to request a download occurs in camera thread.
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
void camera_download (time_t time)
{
   int
      rc;

   printf("%s: download time is: %ld \n", __func__, time);

   rc = write (socks[0], &time, sizeof(time));

   if (rc != sizeof(time)) {
      printf("%s: error writing to socket.: %s\n", __func__, strerror(errno));
      exit(1);
   }

}

/*-------------------------------------------------------------------------
 *
 * name:        _camera_download
 *
 * description: implementation to download footage for a given time stamp
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void _camera_download (time_range_t *time_range)
{
   char
      download_filename[128];
   static const char
      *search_log_filename = "/tmp/search.log";
   file_entry_t
      *fe;

   printf("%s: download start_time: %ld end_time: %ld path: %s \n", __func__,
	  time_range->start_time,
	  time_range->end_time,
	  download_path);

   if (time_range->start_time >= time_range->end_time) {
      printf("%s: all downloads are satisfied.\n", __func__);
      return;
   }

   /*
    * Want to apply a range to the time value to catch videos
    */
   display_time(time_range->start_time);
   display_time(time_range->end_time);

   /*
    * Read the list of avialable videos
    */
   curl_read_list_of_videos(ip_address, search_log_filename);

   /*
    * Get a linked list of all videos that are on the camera
    */
   fe = yaml_find_videos(search_log_filename);

   printf("Search for videos between: \n");
   printf("\t\t\t"); display_time(time_range->start_time);
   printf("\t\t\t"); display_time(time_range->end_time);

   /*
    * Process all entries while the time range has not been satisfied.
    */
   while(fe && (time_range->start_time < time_range->end_time) ) {


#if 0
      printf(">>> name: %s (%d %d) \n",
	     fe->name,
	     (fe->start_time <= time_range->start_time),
	     (fe->end_time >= time_range->start_time));
      display_time(fe->start_time);
      display_time(fe->end_time);

#endif

      if ((fe->end_time <= time_range->start_time) || (fe->start_time >= time_range->end_time) ) {
	 ;
      }
      else {

	 printf("File: %s\n", fe->name);
	 printf("Size: %7.1f MB\n", (float) fe->size / 1000000.0);
	 printf("Start Time: "); display_time(fe->start_time);
	 printf("End   Time: "); display_time(fe->end_time);

	 /*
	  * Generate the name of the file in the local store
	  */
	 snprintf(download_filename, sizeof(download_filename), "%s/%s", download_path, fe->name);

	 /*
	  * See if the file has already been downloaded.
	  */
	 if (file_already_downloaded(download_filename, fe->size)) {
	    printf("*** already downloaded ***\n");
	 }
	 else {
	    printf("Downloading file: %s\n", fe->name);
	    curl_download_file(ip_address, fe->name, download_filename);
	 }

	 /*
	  * Our new start time is the end of this video.
	  */
	 time_range->start_time = fe->end_time + 1;
      }

      fe = fe->next;
   }
}

/*-------------------------------------------------------------------------
 *
 * name:        camera_init
 *
 * description: Get camera thread created, and pass a time stamp to it.
 *
 * input:       download_time - if non-zero, then a time stamp to download
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
void camera_init (time_t download_time, uint32_t download_range, char *_download_path)
{
   int
      pid,
      rc;

   /*
    * If no camera IP address, then cannot do anything
    */
   if (ip_address == NULL) {
      printf("%s: no camera IP address specified\n", __func__);
      return;
   }

   download_path = _download_path;

   /*
    * Make sure the download path exists.
    */
   mkdir(download_path, 0755);

   rc = socketpair(AF_UNIX, SOCK_STREAM, 0, socks);

   if (rc) {
      printf("Error creating socket pair: %s\n", strerror(errno) );
      exit(1);
   }

   pid = fork();

   if (pid < 0) { /* error*/
      printf("Error forking camera thread\n");
      exit(1);
   }

   if (pid) { /* parent process returns */

      if (download_time) {
	 camera_download(download_time);
      }

      return;
   }

   /*
    * Causse child process to die if the parent should die.
    */
   prctl(PR_SET_PDEATHSIG, SIGTERM);

   /*
    * Continue on with the child
    */
   camera_thread(download_time, download_range);
}

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
void camera_ip_address (char *_ip_address)
{
   ip_address = strdup(_ip_address);
}

/*-------------------------------------------------------------------------
 *
 * name:        camera_thread
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void camera_thread (time_t download_time, uint32_t download_range)
{
   int
      rc;
   uint32_t
      done = 0;
   struct pollfd
      fds[1];
   time_range_t
      time_range;

   memset(&time_range, 0, sizeof(time_range));

   /*
    * Setup stuct to return when the socket has data to read.
    */
   fds[0].fd     = socks[1];
   fds[0].events = POLLIN;

   while(!done) {

//      sleep(1);
//      rc = read(socks[0], &download_time, sizeof(download_time) );
//      printf("HACK read returned: %d\n", rc);

      /*
       * wait for a new request, or wakeup every 15 seconds
       */
      rc = poll(fds, 1, 15 * 1000);

      if (rc) {
	 rc = read(socks[1], &download_time, sizeof(download_time) );

	 if (rc != sizeof(download_time) ) {
	    printf("%s: Error reading socket: %s\n", __func__, strerror(errno) );
	    exit(1);
	 }
	 else {
	    time_range.start_time = download_time - download_range;
	    time_range.end_time   = download_time + download_range;
	 }
      }

      /*
       * If current time is more than 15 minutes past the event time, then we're likely
       * not going to get anything.  Just cancel the request.
       */
      check_for_video_timeout(&time_range);

      if (time_range.start_time != time_range.end_time) {
	 _camera_download(&time_range);
      }

   }
}

static void check_for_video_timeout (time_range_t *time_range)
{
   time_t
      tim;

   /*
    * if the time range is already blank, then nothing to do.
    */
   if (!time_range->start_time) {
      return;
   }

   /*
    * Get current time.
    */
   tim = time(NULL);

   if ((tim - time_range->end_time) > (15 * 60)) {
      printf("Timeout waiting for video\n");
      time_range->start_time = time_range->end_time = 0;
   }
}

static void display_time(time_t download_time)
{
   struct tm *tm;

   tm = localtime(&download_time);

   printf("%ld = %d/%d/%d  %2d:%d:%2d (%ld) \n",
	  download_time,
	  tm->tm_mon + 1,
	  tm->tm_mday,
	  tm->tm_year + 1900,
	  tm->tm_hour,
	  tm->tm_min,
	  tm->tm_sec,
      download_time);

}

static uint32_t file_already_downloaded(const char *file_name, uint32_t file_size)
{
   int
      rc;
   struct stat
      sb;

   rc = stat(file_name, &sb);

   if (rc == 0) {
      if (sb.st_size == file_size) {
	 return 1;
      }
   }

   return 0;
}
