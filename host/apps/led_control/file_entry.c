#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "file_entry.h"

static file_entry_t
    *first_file_entry,
    *last_file_entry;

static void do_name ( file_entry_t *fe, char *value_str)
{
   strcpy(fe->name, value_str);
}
static void do_size ( file_entry_t *fe, char *value_str)
{
   fe->size = strtoull(value_str, NULL, 0);
}

static void do_time (time_val_t *tv, char *value);

static void do_end_time (file_entry_t *fe, char *value_str)
{
   do_time(&fe->tv_end_time, value_str);
}

static void do_start_time (file_entry_t *fe, char *value_str)
{
   do_time(&fe->tv_start_time, value_str);
}

static time_fields_t time_fields[] = {
   { "mon",  offsetof(time_val_t, mon)  },
   { "day",  offsetof(time_val_t, day)  },
   { "year", offsetof(time_val_t, year) },
   { "hour", offsetof(time_val_t, hour) },
   { "min",  offsetof(time_val_t, min)  },
   { "sec",  offsetof(time_val_t, sec)  }
};

#define NUMBER_TIME_FIELDS ARRAY_SIZE(time_fields)

static void do_time (time_val_t *tv, char *value_str)
{
   char
      *cp;
   uint32_t
      i,
      idx,
      *ip;
   time_fields_t
      *tf;

   (void) tv;

   cp = strtok(NULL, "");

   for (i=0, tf = time_fields; i < NUMBER_TIME_FIELDS; i++, tf++) {

     if (!strcmp(cp, time_fields[i].name)) {

	ip = (uint32_t *) tv;
	idx = tf->offset / sizeof(uint32_t);

	ip[idx] = atoi(value_str);
	break;
     }
   }
}

/*-------------------------------------------------------------------------
 *
 * name:        file_entry_alloc
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
file_entry_t *file_entry_alloc (void)
{
   file_entry_t
      *fe = calloc(1, sizeof(file_entry_t));

   if (fe == NULL) {
      printf("%s: Error allocating new file entry\n", __func__);
      exit(1);
   }

   if (first_file_entry == NULL) {
      first_file_entry = fe;
   }
   else {
      last_file_entry->next = fe;
   }

   last_file_entry = fe;

   return fe;
}

/*-------------------------------------------------------------------------
 *
 * name:        file_entry_head
 *
 * description: return pointer to first element of the list.
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
file_entry_t *file_entry_head (void)
{
   return first_file_entry;
}

static callback_t
    file_attribute_callbacks[] = {
       { "name",      do_name       },
       { "size",      do_size       },
       { "EndTime",   do_end_time   },
       { "StartTime", do_start_time },
};

/*-------------------------------------------------------------------------
 *
 * name:        file_entry_parse
 *
 * description: called from yaml.c to parse the key/value pairs that we
 *              know are in a 'file' node.
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
void file_entry_parse (file_entry_t *fe, char *key, char *value_str)
{
   char
      *cp;
   uint32_t
      i;
   callback_t
      *cb;

   (void) fe;

   /*
    * We must have at least 3 valid tokens.
    */
   cp = strtok(key, ".");
   if (!cp || strcmp(cp, "value")) { return; }

   cp = strtok(NULL, ".");
   if (!cp || strcmp(cp, "SearchResult")) { return; }

   cp = strtok(NULL, ".");
   if (!cp || strcmp(cp, "File")) { return; }

   cp = strtok(NULL, ".");
   if (!cp) { return; }

   /*
    * Look for item in array of file attributes
    */
   for (i=0,cb = file_attribute_callbacks; i< ARRAY_SIZE(file_attribute_callbacks); i++, cb++) {
      if (!strcmp(cp, cb->name)) {
	 (cb->func)(fe, value_str);
	 return;
      }
   }

}
