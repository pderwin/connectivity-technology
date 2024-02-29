#pragma once

#include <sys/time.h>

void camera_download   (time_t time);
void camera_init       (time_t download_time, uint32_t download_range, char *download_path);
void camera_ip_address (char *ip_address );
