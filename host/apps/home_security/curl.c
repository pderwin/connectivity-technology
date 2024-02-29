#include <errno.h>
#include <curl/curl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "camera.h"

/*-------------------------------------------------------------------------
 *
 * name:        curl_download_file
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
int curl_download_file (const char *ip_address, const char *filename, const char *download_filename)
{
   char
      url[128];
   CURL
      *curl = curl_easy_init();
   CURLcode
      res;
   FILE
      *mp4_fp;

   if (!curl) {
      printf("%s: error creating curl \n", __func__);
      exit(1);
   }

   curl_global_init(CURL_GLOBAL_ALL);

   sprintf(url, "http://%s/cgi-bin/api.cgi?cmd=download&source=%s&user=admin&password=lucky123",
	   ip_address,
	   filename);

//   printf("URL: '%s' \n", url);

   if ((mp4_fp = fopen(download_filename, "w")) == NULL) {
      printf("%s: Error opening search lof for output \n", __func__);
      exit(1);
   }

   curl_easy_setopt(curl, CURLOPT_URL, url);

   /* we want the body be written to this file handle instead of stdout */
   curl_easy_setopt(curl, CURLOPT_WRITEDATA, mp4_fp);

   res = curl_easy_perform(curl);

   if(res != CURLE_OK) {
      printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
   }

   fclose(mp4_fp);

   printf("Download complete\n");

   return 0;
}

/*-------------------------------------------------------------------------
 *
 * name:        curl_read_list_of_videos
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
void curl_read_list_of_videos(char *ip_address, const char *filename)
{
   char
      url[128];
   CURL
      *curl = curl_easy_init();
   CURLcode
      res;
   FILE
      *search_fp;

   if (!curl) {
      printf("%s: error creating curl \n", __func__);
      exit(1);
   }

   curl_global_init(CURL_GLOBAL_ALL);

   sprintf(url, "http://%s/cgi-bin/api.cgi?user=admin&password=lucky123&cmd=Search", ip_address);

//   printf("URL: '%s' \n", url);

   curl_easy_setopt(curl, CURLOPT_URL, url);

   if ((search_fp = fopen(filename, "w")) == NULL) {
      printf("%s: Error opening search lof for output \n", __func__);
      exit(1);
   }

   /* we want the body be written to this file handle instead of stdout */
   curl_easy_setopt(curl, CURLOPT_WRITEDATA, search_fp);

   res = curl_easy_perform(curl);

   if(res != CURLE_OK) {
      printf("curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
   }

   fclose(search_fp);

   /* always cleanup */
   curl_easy_cleanup(curl);
}
