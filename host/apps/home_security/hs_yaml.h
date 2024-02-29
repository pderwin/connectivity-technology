#pragma once

#include <time.h>
#include "file_entry.h"

typedef struct {
   uint32_t
       type;
   char
       value[100];
} event_t;

file_entry_t
    *yaml_find_videos (const char *filename);
