#include <stdint.h>
#include "event.h"
#include "yaml.h"
#include "up.h"

/*-------------------------------------------------------------------------
 *
 * name:        up_parse
 *
 * description: parse the YAML string in the payload, searching for key/value
 *              items and call event routines to further process.
 *
 * input:
 *
 * output:
 *
 *-------------------------------------------------------------------------*/
void up_parse(uint8_t *payload, uint32_t payload_len)
{
   char
      key[80],
      value[80];
   uint32_t
      done = 0,
      last_token_type = 0;
   yaml_parser_t
      parser;
   yaml_token_t
      token;

   /*
    * If no payload then nothing to do.
    */
   if (payload_len == 0) {
      return;
   }

   /*
    * Init the event structure.
    */
   event_init();

   yaml_parser_initialize(&parser);

   yaml_parser_set_input_string(&parser, payload, payload_len);

   while ( yaml_parser_scan(&parser, &token) && !done) {

      switch(token.type) {

	  case YAML_STREAM_START_TOKEN:
	     break;

	  case YAML_STREAM_END_TOKEN:
	     done = 1;
	     break;

	     /* Token types (read before actual token) */
	  case YAML_KEY_TOKEN:
	     last_token_type = token.type;
	     break;

	  case YAML_VALUE_TOKEN:
	     last_token_type = token.type;
	     break;

	  case YAML_SCALAR_TOKEN:
	     switch(last_token_type) {
		 case YAML_KEY_TOKEN:
		    strcpy(key, (char *) token.data.scalar.value);
		    break;
		 case YAML_VALUE_TOKEN:
		    strcpy(value, (char *) token.data.scalar.value);

		    event_key_value(key, value);

		    break;
	     }
	     last_token_type = YAML_NO_TOKEN;
	     break;

	     /* Block delimeters */
	  case YAML_BLOCK_SEQUENCE_START_TOKEN: puts("<b>Start Block (Sequence)</b>"); break;
	  case YAML_BLOCK_ENTRY_TOKEN:          puts("<b>Start Block (Entry)</b>");    break;
	  case YAML_BLOCK_END_TOKEN:            puts("<b>End block</b>");              break;
	     /* Data */
	  case YAML_BLOCK_MAPPING_START_TOKEN:  puts("[Block mapping]");            break;
	     /* Others */
	  default:
      }

      yaml_token_delete(&token);
  }

   yaml_parser_delete(&parser);

   event_process();
}
