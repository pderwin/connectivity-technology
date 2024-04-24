#pragma once

void mqtt_init (void);
void mqtt_publish (char *topic_name, char *payload, uint32_t payload_len);
