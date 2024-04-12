#pragma once

/*
 * enumerate priorities.  A smaller number has higher precedence over higher numbers.
 */
typedef enum {
   PRIORITY_BUTTONS,
   PRIORITY_SEMTRACKER,
   PRIORITY_LED,
   PRIORITY_PIR,
   PRIORITY_DRIVEWAY_SENSOR
} priority_e;
