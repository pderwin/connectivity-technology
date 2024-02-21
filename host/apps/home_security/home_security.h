#pragma once

typedef struct {
   char* connection;         /**< connection to system under test. */
   char** haconnections;
   char* proxy_connection;
   int hacount;
   int verbose;
   int test_no;
   int MQTTVersion;
   int iterations;
} options_t;
