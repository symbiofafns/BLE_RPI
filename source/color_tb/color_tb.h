#ifndef _COLOR_TABLE_
#define _COLOR_TABLE_

#include <stdint.h>


union color_t
{
	uint8_t v[3];
	struct
	{
		uint8_t r;
		uint8_t g;
		uint8_t b;
	}member;
};

union color_t get_temperature_rgb_value(int16_t);

#endif
