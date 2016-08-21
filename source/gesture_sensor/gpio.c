#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "gpio.h"

#define BUFFER_MAX 		3
#define DIRECTION_MAX 	35
#define VALUE_MAX 		30


int16_t gpio_export(int16_t pin)
{
	char 	buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int16_t fd;
 
	fd = open("/sys/class/gpio/export", O_WRONLY);
	if(fd < 0){
		printf("Failed to open export for writing!\n");
		return(-1);
	}
 
	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}

int16_t gpio_unexport(int16_t pin)
{
	char 	buffer[BUFFER_MAX];
	ssize_t bytes_written;
	int16_t fd;
 
	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if(fd < 0){
		printf("Failed to open unexport for writing!\n");
		return(-1);
	}
 
	bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
	write(fd, buffer, bytes_written);
	close(fd);
	return(0);
}

int16_t gpio_direction(int16_t pin, int16_t dir)
{
	static const char s_directions_str[]  = "in\0out";
 
	char 	path[DIRECTION_MAX];
	int16_t fd;
 
	snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
	fd = open(path, O_WRONLY);
	if(fd < 0){
		printf("Failed to open gpio direction for writing!\n");
		return(-1);
	}
 
	if (write(fd, &s_directions_str[INPUT == dir ? 0 : 3], INPUT == dir ? 2 : 3) < 0){
		printf("Failed to set direction!\n");
		return(-1);
	} 
	close(fd);
	return(0);
}

int16_t gpio_read(int16_t pin)
{
	char 	path[VALUE_MAX];
	char 	value_str[3];
	int16_t fd;
 
	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_RDONLY);
	if(fd < 0){
		printf("Failed to open gpio value for reading!\n");
		return(-1);
	}
 
	if(read(fd, value_str, 3) < 0){
		printf("Failed to read value!\n");
		return(-1);
	}
 	close(fd);
 	return(atoi(value_str));
}

int16_t gpio_write(int16_t pin, int16_t value)
{
	static const char s_values_str[] = "01";
 
	char 	path[VALUE_MAX];
	int16_t	fd;
 
	snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
	fd = open(path, O_WRONLY);
	if(fd < 0){
		printf("Failed to open gpio value for writing!\n");
		return(-1);
	}
 
	if (write(fd, &s_values_str[LOW == value ? 0 : 1], 1) != 1){
		printf("Failed to write value!\n");
		return(-1);
	} 
	close(fd);
	return(0);
}
