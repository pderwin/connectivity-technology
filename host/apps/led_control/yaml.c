#include <stdint.h>
#include "file_entry.h"
#include "hs_yaml.h"
#include "yaml.h"

#define MAX_TOKEN_STACK  (10)
#define MAX_TOKEN_LENGTH (20)

#if (0)
static char
    *event_strs[] = {
      "YAML_NO_EVENT",
      "YAML_STREAM_START_EVENT",
      "YAML_STREAM_END_EVENT",
      "YAML_DOCUMENT_START_EVENT",
      "YAML_DOCUMENT_END_EVENT",
      "YAML_ALIAS_EVENT",
      "YAML_SCALAR_EVENT",
      "YAML_SEQUENCE_START_EVENT",
      "YAML_SEQUENCE_END_EVENT",
      "YAML_MAPPING_START_EVENT",
      "YAML_MAPPING_END_EVENT"
};
#endif

static char
    tokens[MAX_TOKEN_STACK][MAX_TOKEN_LENGTH];

static event_t
    my_unget_event;
static uint32_t
    unget_valid,
    valid_tokens;

static time_t make_time_t    (time_val_t *tv);
static void   parse_events   (yaml_parser_t *parser, file_entry_t *file_entry);
static void   parse_sequence (yaml_parser_t *parser);

#define get_event(__p, __e) _get_event(__p, __e, __LINE__)

static void _get_event(yaml_parser_t *parser, event_t *event, uint32_t lineno)
{
   yaml_event_t
      yaml_event;

   (void) lineno;

   if (unget_valid) {
      memcpy(event, &my_unget_event, sizeof(event_t));
      unget_valid = 0;
   }
   else {
      yaml_parser_parse(parser, &yaml_event);

      event->type = yaml_event.type;
      if (yaml_event.type == YAML_SCALAR_EVENT) {
	 strcpy(event->value, (char *) yaml_event.data.scalar.value);
      }
      else {
	 strcpy(event->value, "N\\A");
      }

      yaml_event_delete(&yaml_event);
   }

//   printf("L %3d %*s type: %d %s\n", lineno, (valid_tokens * 5), "", event->type, event_strs[event->type] );
}

static void unget_event(event_t *event)
{
   memcpy(&my_unget_event, event, sizeof(unget_event));
   unget_valid = 1;
}

static void parse_events (yaml_parser_t *parser, file_entry_t *fe)
{
   uint32_t
      i;
   event_t
      event;

   get_event(parser, &event);

   switch(event.type) {
       case YAML_MAPPING_START_EVENT:

	  get_event(parser, &event);

	  while(event.type != YAML_MAPPING_END_EVENT) {
	     /*
	      * This should be a scalar giving the name of a value.
	      */
	     if (event.type != YAML_SCALAR_EVENT) {
		printf("ERROR: unexpected scalar type\n");
		return;
	     }

	     strcpy(tokens[valid_tokens], event.value);
	     valid_tokens++;

	     parse_events(parser, fe);

	     valid_tokens--;

	     get_event(parser, &event);
	  }

	  break;

       case YAML_SCALAR_EVENT:
       {
	  char key[100];

	  key[0] = 0;

	  for (i=0; i<valid_tokens; i++) {
	     if (i) {
		strcat(key,".");
	     }
	     strcat(key, tokens[i]);
	  }

	  file_entry_parse(fe, key, event.value);
       }
       break;


       case YAML_SEQUENCE_START_EVENT:
	  parse_sequence(parser);
	  break;

       default:
	  break;
   }
}

/*-------------------------------------------------------------------------
 *
 * name:        parse_sequence
 *
 * description: parse a 'sequence' which represents a file on the SD card.
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static void parse_sequence (yaml_parser_t *parser)
{
   event_t
      event;
   file_entry_t
      *fe;

   while(1) {

//      printf("\n\n\n========================\n");

      fe = file_entry_alloc();

      parse_events(parser, fe);


//      printf("Start: %d/%d/%d %d:%d:%d \n",
//	     fe->start_time.mon,
//	     fe->start_time.day,
//	     fe->start_time.year,
//	     fe->start_time.hour,
//	     fe->start_time.min,
//	     fe->start_time.sec);

//      printf("End:   %d/%d/%d %d:%d:%d \n",
//	     fe->end_time.mon,
//	     fe->end_time.day,
//	     fe->end_time.year,
//	     fe->end_time.hour,
//	     fe->end_time.min,
//	     fe->end_time.sec);

//      printf("name: %s\n", fe->name);
//      printf("size: %d\n", fe->size);

      /*
       * Convert the tm struct into just a single time_t value for easy comparisons.
       */
      fe->start_time = make_time_t(&fe->tv_start_time);
      fe->end_time   = make_time_t(&fe->tv_end_time);

//      printf("=====================\n\n\n");
      /*
       * Get the next event and see if it is a SEQUENCE END.
       */
      get_event(parser, &event);

      if (event.type == YAML_SEQUENCE_END_EVENT) {
//	 printf("Got the SEQUENCE END.\n");
	 break;
      }
      else {
	 unget_event(&event);
      }
   }

//   printf("Done handling sequence start\n\n\n\n");
}

/*-------------------------------------------------------------------------
 *
 * name:        make_time_t
 *
 * description:
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
static time_t make_time_t (time_val_t *tv)
{
   struct tm tm;

   memset(&tm, 0, sizeof(tm));

   tm.tm_mon   = tv->mon - 1;
   tm.tm_mday  = tv->day;
   tm.tm_year  = tv->year - 1900;

   tm.tm_hour  = tv->hour;
   tm.tm_min   = tv->min;
   tm.tm_sec   = tv->sec;

   tm.tm_isdst = -1;  // attempt to use system databases to figure out if DST

   return mktime(&tm);

//   printf("Converted time: %ld \n", tv->time);
}


/*-------------------------------------------------------------------------
 *
 * name:        yaml_find_video
 *
 * description: main entry in this file.  Find a video which meets the
 *              given time stamp.
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
file_entry_t *yaml_find_videos(const char *filename)
{
   FILE
      *fp;
   yaml_parser_t
      parser;
   event_t
      event;

   fp = fopen(filename, "r");

   yaml_parser_initialize(&parser);

   yaml_parser_set_input_file(&parser, fp);

   /*
    * Bypass the stream start.
    */
   get_event(&parser, &event);

   /*
    * Bypass the document start.
    */
   get_event(&parser, &event);
   get_event(&parser, &event);

   valid_tokens = 0;

   parse_events(&parser, NULL);

   yaml_parser_delete(&parser);

   /*
    * return the list of files.
    */
   return file_entry_head();
}
