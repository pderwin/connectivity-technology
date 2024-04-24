#pragma once

#include <stdint.h>
#include <time.h>

#define ARRAY_SIZE(__var) ( sizeof(__var) / sizeof(__var[0]) )

typedef struct file_entry_s file_entry_t;

typedef struct {
   uint32_t
       hour,
       min,
       sec;
   uint32_t
       mon,
       day,
       year;
} time_val_t;

struct file_entry_s {
   uint32_t
       idx;
   char
       name[80];
   uint32_t
       size;

   /*
    * Start and End time expressed in seconds
    */
   time_t
       start_time,
       end_time;

   /*
    * Broken down time value.
    */
   time_val_t
       tv_start_time,
       tv_end_time;

   file_entry_t
       *next;
};

typedef struct {
   char *name;
   void (*func)(file_entry_t *fe, char *value_str);
} callback_t;

typedef struct {
   char     *name;
   uint32_t offset;
} time_fields_t;

file_entry_t *file_entry_alloc      (void);
file_entry_t *file_entry_delete_all (void);
file_entry_t *file_entry_head       (void);
void          file_entry_parse      (file_entry_t *fe, char *key, char *value);
