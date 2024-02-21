#pragma once

#include <stdint.h>

unsigned char *base64_decode(const unsigned char *data,
			     uint32_t input_length,
			     uint32_t *output_length);
