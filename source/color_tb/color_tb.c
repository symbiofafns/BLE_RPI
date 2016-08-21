#include "color_tb.h"

#define TEMPERATURE_MIN					10
#define TEMPERATURE_MAX					40


const union color_t rgb_table[]=
{
	{255,	51,		0},		// 40 degC
	{255,	83,		0},		
	{255,	102,	0},	
	{255,	118,	0},
	{255,	126,	0},
	{255,	137,	18},
	{255,	152,	41},
	{255,	162,	60},
	{255,	170,	77},
	{255,	173,	94},
	{255,	185,	105},
	{255,	213,	161},
	{255,	217,	171},
	{255,	221,	180},
	{255,	255,	188},
	{255,	228,	196},
	{255,	228,	206},
	{255,	234,	211},
	{255,	237,	218},
	{255,	239,	225},
	{255,	241,	231},
	{255,	243,	239},
	{247,	245,	255},
	{239,	240,	255},
	{233,	237,	255},
	{228,	234,	255},
	{224,	231,	255},
	{223,	229,	255},
	{219,	226,	255},
	{207,	218,	255},
	{155,	188,	255} 	// 10 degC
}; 

union color_t get_temperature_rgb_value(int16_t t)
{
	union color_t res;
	uint8_t		  index;
	
	if(t <= TEMPERATURE_MIN){
		t = TEMPERATURE_MIN;
	}else if(t >= TEMPERATURE_MAX){
		t = TEMPERATURE_MAX;
	}
	
	index = TEMPERATURE_MAX - t;
	return(rgb_table[index]);
}
