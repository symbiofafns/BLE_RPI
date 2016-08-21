#ifndef _GPIO_
#define _GPIO_

#include <stdint.h>

#define INPUT 	0
#define OUTPUT 	1
 
#define LOW  	0
#define HIGH 	1

int16_t gpio_export		(int16_t);
int16_t gpio_direction	(int16_t,int16_t);
int16_t gpio_read		(int16_t);
int16_t gpio_write		(int16_t,int16_t);


#endif
