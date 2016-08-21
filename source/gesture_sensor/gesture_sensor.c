#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>				
#include <fcntl.h>				
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include "gesture_sensor.h"
#include "gpio.h"

#define INT_PIN_GPIO	4

static
int16_t i2c_fd;

static
struct gesture_data_t	gestrue_data_variable;


//======================== sub function ===========================================
static
int16_t i2c_device_initial(void)
{
	char 	*filename = (char*)"/dev/i2c-1";
	
	//----- OPEN THE I2C BUS -----
	if((i2c_fd = open(filename, O_RDWR)) < 0){
		printf("Failed to open the i2c bus\n");
		return(-1);
	}
	//----- Set I2C BUS Slave Address -----
	if(ioctl(i2c_fd, I2C_SLAVE, APDS9960_I2C_ADDR) < 0){
		printf("Failed to acquire bus access and/or talk to slave.\n");
		return(-1);
	}	
	return(0);
}


static 
int16_t write_register(uint8_t reg_name, uint8_t val)
{
	uint8_t buffer[2];

    buffer[0] = reg_name;
    buffer[1] = val;
    if(write(i2c_fd, buffer, 2) != 2){
		return(-1);
	}
    return(0);
}

static
int16_t read_register(uint8_t reg_name, uint8_t *val)
{
	uint8_t buffer[2];
    buffer[0] = reg_name;
    if(write(i2c_fd, buffer, 1) != 1){
		return(-1);
	}
	if(read(i2c_fd,   buffer,1) != 1){
        return(-1);
	}
    *val = buffer[0];
	return(0);
}

static
int16_t read_data_block(uint8_t reg_name, uint8_t *val, int16_t size)
{
	uint8_t buffer[2];

    buffer[0] = reg_name;
	if(write(i2c_fd, buffer, 1) != 1){
		return(-1);
	}
	if(read(i2c_fd,  val, size) != size){
		return(-1);
	}
	return(0);
}

//======================== sub function(APDS-9960) ===========================================
static 
int16_t get_mode(void)
{
	uint8_t enable_value;
	
	/* Read current ENABLE register */
	if( read_register(APDS9960_ENABLE, &enable_value) != 0) {
		return(-1);
	}
		
	return enable_value;	
}

static
int16_t set_mode(uint8_t mode, uint8_t enable)
{
	uint8_t reg_val;
	int16_t res;

	/* Read current ENABLE register */
	res = get_mode();
	if( res == -1 != 0){
		return(-1);
	}
	reg_val = (uint8_t)(res);
	
	/* Change bit(s) in ENABLE register */
	enable = enable & 0x01;
	if( mode >= 0 && mode <= 6 != 0){
		if (enable) {
			reg_val |= (1 << mode);
		} else {
			reg_val &= ~(1 << mode);
		}
	} else if( mode == ALL != 0){
		if (enable) {
			reg_val = 0x7F;
		} else {
			reg_val = 0x00;
		}
	}
	
	/* Write value back to ENABLE register */
	if( write_register(APDS9960_ENABLE, reg_val) != 0){
		return(-1);
	}
	
	return(0);	
}

/**
* @brief Sets the LED drive strength for proximity and ALS
*
* Value    LED Current
*   0        100 mA
*   1         50 mA
*   2         25 mA
*   3         12.5 mA
*
* @param[in] drive the value (0-3) for the LED drive strength
* @return True if operation successful. False otherwise.
*/
static
int16_t set_led_drive(uint8_t drive)
{
	uint8_t val;
	
	/* Read value from CONTROL register */
	if( read_register(APDS9960_CONTROL, &val) != 0){
		return(-1);
	}
	
	/* Set bits in register to given value */
	drive &= 0b00000011;
	drive  = drive << 6;
	val   &= 0b00111111;
	val   |= drive;
	
	/* Write register value back into CONTROL register */
	if( write_register(APDS9960_CONTROL, val) != 0){
		return(-1);
	}
	
	return(0);
}

/**
* @brief Sets the receiver gain for proximity detection
*
* Value    Gain
*   0       1x
*   1       2x
*   2       4x
*   3       8x
*
* @param[in] drive the value (0-3) for the gain
* @return True if operation successful. False otherwise.
*/
static
int16_t set_proximity_gain(uint8_t drive)
{
	uint8_t val;
	
	/* Read value from CONTROL register */
	if( read_register(APDS9960_CONTROL, &val) != 0){
		return(-1);
	}
	
	/* Set bits in register to given value */
	drive &= 0b00000011;
	drive  = drive << 2;
	val   &= 0b11110011;
	val   |= drive;
	
	/* Write register value back into CONTROL register */
	if( write_register(APDS9960_CONTROL, val) != 0){
		return(-1);
	}
	
	return(0);
}

/**
* @brief Sets the receiver gain for the ambient light sensor (ALS)
*
* Value    Gain
*   0        1x
*   1        4x
*   2       16x
*   3       64x
*
* @param[in] drive the value (0-3) for the gain
* @return True if operation successful. False otherwise.
*/
static
int16_t set_ambient_light_gain(uint8_t drive)
{
	uint8_t val;
	
	/* Read value from CONTROL register */
	if( read_register(APDS9960_CONTROL, &val) != 0){
		return(-1);
	}
	
	/* Set bits in register to given value */
	drive &= 0b00000011;
	val &= 0b11111100;
	val |= drive;
	
	/* Write register value back into CONTROL register */
	if( write_register(APDS9960_CONTROL, val) != 0){
		return(-1);
	}
	
	return(0);
}

static
int16_t set_prox_int_low_thresh(uint8_t threshold)
{
	if( write_register(APDS9960_PILT, threshold) != 0){
		return(-1);
	}
	
	return(0);
}

static
int16_t set_prox_int_high_thresh(uint8_t threshold)
{
	if( write_register(APDS9960_PIHT, threshold) != 0){
		return(-1);
	}
	
	return(0);
}

static
int16_t set_light_int_low_threshold(uint16_t threshold)
{
	uint8_t val_low;
	uint8_t val_high;
	
	/* Break 16-bit threshold into 2 8-bit values */
	val_low = threshold & 0x00FF;
	val_high = (threshold & 0xFF00) >> 8;
	
	/* Write low byte */
	if( write_register(APDS9960_AILTL, val_low) != 0){
		return(-1);
	}
	
	/* Write high byte */
	if( write_register(APDS9960_AILTH, val_high) != 0){
		return(-1);
	}
	
	return(0);
}

static
int16_t set_light_int_high_threshold(uint16_t threshold)
{
	uint8_t val_low;
	uint8_t val_high;
	
	/* Break 16-bit threshold into 2 8-bit values */
	val_low = threshold & 0x00FF;
	val_high = (threshold & 0xFF00) >> 8;
	
	/* Write low byte */
	if( write_register(APDS9960_AIHTL, val_low) != 0){
		return(-1);
	}
	
	/* Write high byte */
	if( write_register(APDS9960_AIHTH, val_high) != 0){
		return(-1);
	}
	
	return(0);
}

static
int16_t set_gesture_enter_thresh(uint8_t threshold)
{
	if( write_register(APDS9960_GPENTH, threshold) != 0){
		return(-1);
	}
	
	return(0);
}

static
int16_t set_gesture_exit_thresh(uint8_t threshold)
{
	if( write_register(APDS9960_GEXTH, threshold) != 0){
		return(-1);
	}
	
	return(0);
}

/**
* @brief Sets the gain of the photodiode during gesture mode
*
* Value    Gain
*   0       1x
*   1       2x
*   2       4x
*   3       8x
*
* @param[in] gain the value for the photodiode gain
* @return True if operation successful. False otherwise.
*/
static
int16_t set_gesture_gain(uint8_t gain)
{
	uint8_t val;
	
	/* Read value from GCONF2 register */
	if( read_register(APDS9960_GCONF2, &val) != 0){
		return(-1);
	}
	
	/* Set bits in register to given value */
	gain &= 0b00000011;
	gain = gain << 5;
	val &= 0b10011111;
	val |= gain;
	
	/* Write register value back into GCONF2 register */
	if( write_register(APDS9960_GCONF2, val) != 0){
		return(-1);
	}
	
	return(0);
}

/**
* @brief Sets the LED drive current during gesture mode
*
* Value    LED Current
*   0        100 mA
*   1         50 mA
*   2         25 mA
*   3         12.5 mA
*
* @param[in] drive the value for the LED drive current
* @return True if operation successful. False otherwise.
*/
static
int16_t set_gesture_led_drive(uint8_t drive)
{
	uint8_t val;
	
	/* Read value from GCONF2 register */
	if( read_register(APDS9960_GCONF2, &val) != 0){
		return(-1);
	}
	
	/* Set bits in register to given value */
	drive &= 0b00000011;
	drive = drive << 3;
	val &= 0b11100111;
	val |= drive;
	
	/* Write register value back into GCONF2 register */
	if( write_register(APDS9960_GCONF2, val) != 0){
		return(-1);
	}
	
	return(0);
}

/**
* @brief Sets the time in low power mode between gesture detections
*
* Value    Wait time
*   0          0 ms
*   1          2.8 ms
*   2          5.6 ms
*   3          8.4 ms
*   4         14.0 ms
*   5         22.4 ms
*   6         30.8 ms
*   7         39.2 ms
*
* @param[in] the value for the wait time
* @return True if operation successful. False otherwise.
*/
static
int16_t set_gesture_wait_time(uint8_t time)
{
	uint8_t val;
	
	/* Read value from GCONF2 register */
	if( read_register(APDS9960_GCONF2, &val) != 0){
		return(-1);
	}
	
	/* Set bits in register to given value */
	time &= 0b00000111;
	val &= 0b11111000;
	val |= time;
	
	/* Write register value back into GCONF2 register */
	if( write_register(APDS9960_GCONF2, val) != 0){
		return(-1);
	}
	
	return(0);
}

/**
* @brief Sets the LED current boost value
*
* Value  Boost Current
*   0        100%
*   1        150%
*   2        200%
*   3        300%
*
* @param[in] drive the value (0-3) for current boost (100-300%)
* @return True if operation successful. False otherwise.
*/
int16_t set_led_boost(uint8_t boost)
{
	uint8_t val;
	
	/* Read value from CONFIG2 register */
	if( read_register(APDS9960_CONFIG2, &val) != 0){
		return(-1);
	}
	
	/* Set bits in register to given value */
	boost &= 0b00000011;
	boost = boost << 4;
	val &= 0b11001111;
	val |= boost;
	
	/* Write register value back into CONFIG2 register */
	if( write_register(APDS9960_CONFIG2, val) != 0){
		return(-1);
	}
	
	return(0);
}  

static
int16_t set_gesture_mode(uint8_t mode)
{
	uint8_t val;
	
	/* Read value from GCONF4 register */
	if( read_register(APDS9960_GCONF4, &val) != 0){
		return(-1);
	}
	
	/* Set bits in register to given value */
	mode &= 0b00000001;
	val &= 0b11111110;
	val |= mode;
	
	/* Write register value back into GCONF4 register */
	if( write_register(APDS9960_GCONF4, val) != 0){
		return(-1);
	}
	
	return(0);
}  

static
int16_t set_gesture_int_enable(uint8_t enable)
{
	uint8_t val;
	
	/* Read value from GCONF4 register */
	if( read_register(APDS9960_GCONF4, &val) != 0){
		return(-1);
	}
	
	/* Set bits in register to given value */
	enable &= 0b00000001;
	enable = enable << 1;
	val &= 0b11111101;
	val |= enable;
	
	/* Write register value back into GCONF4 register */
	if( write_register(APDS9960_GCONF4, val) != 0){
		return(-1);
	}
	
	return(0);
}

static
int16_t enable_power(void)
{
	if( set_mode(POWER, 1) != 0){
		return(-1);
	}
		
	return(0);
}

static
int16_t enable_gesture_sensor(uint8_t interrupts)
{
	
	/* Enable gesture mode
		Set ENABLE to 0 (power off)
		Set WTIME to 0xFF
		Set AUX to LED_BOOST_300
		Enable PON, WEN, PEN, GEN in ENABLE 
	*/
	
	if( write_register(APDS9960_WTIME, 0xFF) != 0){
		return(-1);
	}
	if( write_register(APDS9960_PPULSE, DEFAULT_GESTURE_PPULSE) != 0){
		return(-1);
	}
	if( set_led_boost(LED_BOOST_300) != 0){
		return(-1);
	}
	if( interrupts != 0){
		if( set_gesture_int_enable(1) != 0){
			return(-1);
		}
	} else {
		if( set_gesture_int_enable(0) != 0){
			return(-1);
		}
	}
	if( set_gesture_mode(1) != 0){
		return(-1);
	}
	if( enable_power() != 0){
		return(-1);
	}
	if( set_mode(WAIT, 1) != 0){
		return(-1);
	}
	if( set_mode(PROXIMITY, 1) != 0){
		return(-1);
	}
	if( set_mode(GESTURE, 1) != 0){
		return(-1);
	}
	
	return(0);
}


static
int16_t gesture_sensor_process_data(void)
{
	uint8_t u_first = 0, d_first = 0, l_first = 0, r_first = 0;
    uint8_t u_last  = 0, d_last  = 0, l_last  = 0, r_last  = 0;
	int16_t ud_ratio_first, lr_ratio_first;
	int16_t ud_ratio_last,  lr_ratio_last;
	int16_t ud_delta,		lr_delta;
	
	uint8_t i;
	
	if(gestrue_data_variable.array_size <= 4){
		return(-1);
	}
	
	if(gestrue_data_variable.array_size <= 32 && gestrue_data_variable.array_size > 0){
		/* Find the first value in U/D/L/R above the threshold */
		for(i=0; i < gestrue_data_variable.array_size; i++){
			if((gestrue_data_variable.value_array[i].up    > GESTURE_THRESHOLD_OUT) &&
			   (gestrue_data_variable.value_array[i].down  > GESTURE_THRESHOLD_OUT) &&
			   (gestrue_data_variable.value_array[i].left  > GESTURE_THRESHOLD_OUT) &&
			   (gestrue_data_variable.value_array[i].right > GESTURE_THRESHOLD_OUT)    ){
			   
			   u_first = gestrue_data_variable.value_array[i].up;
			   d_first = gestrue_data_variable.value_array[i].down;
			   l_first = gestrue_data_variable.value_array[i].left;
			   r_first = gestrue_data_variable.value_array[i].right;
			   break;
			}
		}
		
		 /* If one of the _first values is 0, then there is no good data */
		if((u_first == 0) || (d_first == 0) || (l_first == 0) || (r_first == 0)){
			return(-1);
		}
		
		/* Find the last value in U/D/L/R above the threshold */
		for( i= gestrue_data_variable.array_size-1; i >= 0; i--) {
			if((gestrue_data_variable.value_array[i].up    > GESTURE_THRESHOLD_OUT) &&
			   (gestrue_data_variable.value_array[i].down  > GESTURE_THRESHOLD_OUT) &&
			   (gestrue_data_variable.value_array[i].left  > GESTURE_THRESHOLD_OUT) &&
			   (gestrue_data_variable.value_array[i].right > GESTURE_THRESHOLD_OUT)    ){
			   
			   u_last = gestrue_data_variable.value_array[i].up ;
			   d_last = gestrue_data_variable.value_array[i].down;
			   l_last = gestrue_data_variable.value_array[i].left;
			   r_last = gestrue_data_variable.value_array[i].right;
			   break;
			}
		}
		
		/* Calculate the first vs. last ratio of up/down and left/right */
		ud_ratio_first = ((u_first - d_first) * 100) / (u_first + d_first);
		lr_ratio_first = ((l_first - r_first) * 100) / (l_first + r_first);
		ud_ratio_last  = ((u_last  - d_last)  * 100) / (u_last  + d_last);
		lr_ratio_last  = ((l_last  - r_last)  * 100) / (l_last  + r_last);
		
		/* Determine the difference between the first and last ratios */
		ud_delta 	   = ud_ratio_last - ud_ratio_first;
		lr_delta 	   = lr_ratio_last - lr_ratio_first;
		
		/* Accumulate the UD and LR delta values */
		gestrue_data_variable.ud_delta += ud_delta;
		gestrue_data_variable.lr_delta += lr_delta;
	
		/* Determine U/D gesture */
		 if(gestrue_data_variable.ud_delta >= GESTURE_SENSITIVITY_1){
			gestrue_data_variable.ud_count = 1;
		}else if(gestrue_data_variable.ud_delta <= -GESTURE_SENSITIVITY_1 ) {
			gestrue_data_variable.ud_count = -1;
		}else{
			gestrue_data_variable.ud_count = 0;
		}
	
		/* Determine L/R gesture */
		  if(gestrue_data_variable.lr_delta >= GESTURE_SENSITIVITY_1){
			gestrue_data_variable.lr_count = 1;
		}else if(gestrue_data_variable.lr_delta <= -GESTURE_SENSITIVITY_1){
			gestrue_data_variable.lr_count = -1;
		}else{
			gestrue_data_variable.lr_count = 0;
		}
		
		 /* Determine Near/Far gesture */
		if((gestrue_data_variable.ud_count == 0) && (gestrue_data_variable.lr_count == 0)){
			if((abs(ud_delta) < GESTURE_SENSITIVITY_2) && (abs(lr_delta) < GESTURE_SENSITIVITY_2)){
				if((ud_delta == 0) && (lr_delta == 0)){
					gestrue_data_variable.near_count++;
				}else if((ud_delta != 0) || (lr_delta != 0)){
					gestrue_data_variable.far_count++;
				}
				
				if((gestrue_data_variable.near_count >= 10) && (gestrue_data_variable.far_count >= 2)){
					if((ud_delta == 0) && (lr_delta == 0) ){
						gestrue_data_variable.status = Near;
					}else if((ud_delta != 0) && (lr_delta != 0)) {
						gestrue_data_variable.status = Far;
					}
					return(0);
				}				
			}
		}else{
			if((abs(ud_delta) < GESTURE_SENSITIVITY_2) && (abs(lr_delta) < GESTURE_SENSITIVITY_2)){
				if((ud_delta == 0) && (lr_delta == 0)){
					gestrue_data_variable.near_count++;
				}
				
				if(gestrue_data_variable.near_count >= 10){
					gestrue_data_variable.ud_count = 0;
					gestrue_data_variable.lr_count = 0;
					gestrue_data_variable.ud_delta = 0;
					gestrue_data_variable.lr_delta = 0;
				}
			}
		}    
	}
	return(-1);	
}

static
int16_t gesture_sensor_decode(void)
{
	/* Return if near or far event is detected */
	if(gestrue_data_variable.status == Near || gestrue_data_variable.status == Far){
		return(0);
	}
	
	/* Determine swipe direction */
	if(		 (gestrue_data_variable.ud_count == -1) &&  (gestrue_data_variable.lr_count == 0)){
			gestrue_data_variable.status = Up;
	
	}else if((gestrue_data_variable.ud_count ==  1) &&  (gestrue_data_variable.lr_count == 0)){
			gestrue_data_variable.status = Down;
	
	}else if((gestrue_data_variable.ud_count ==  0)  && (gestrue_data_variable.lr_count == 1)){
			gestrue_data_variable.status = Right;

	}else if((gestrue_data_variable.ud_count ==  0)  && (gestrue_data_variable.lr_count == -1)){
			gestrue_data_variable.status = Left;

	}else if((gestrue_data_variable.ud_count == -1)  && (gestrue_data_variable.lr_count == 1)){
		if(abs(gestrue_data_variable.ud_delta) > abs(gestrue_data_variable.lr_delta)){
			gestrue_data_variable.status = Up;
		}else{
			gestrue_data_variable.status = Right;
		}
	
	}else if((gestrue_data_variable.ud_count == 1)  && (gestrue_data_variable.lr_count == -1)){
		if(abs(gestrue_data_variable.ud_delta) > abs(gestrue_data_variable.lr_delta)){
			gestrue_data_variable.status = Down;
		}else {
			gestrue_data_variable.status = Left;
		}
	
	}else if((gestrue_data_variable.ud_count == -1) && (gestrue_data_variable.lr_count == -1)){
		if(abs(gestrue_data_variable.ud_delta) > abs(gestrue_data_variable.lr_delta)){
			gestrue_data_variable.status = Up;
		}else{
			gestrue_data_variable.status = Left;
		}
	
	}else if((gestrue_data_variable.ud_count  == 1) && (gestrue_data_variable.lr_count == 1)){
		if(abs(gestrue_data_variable.ud_delta) > abs(gestrue_data_variable.lr_delta)){
			gestrue_data_variable.status = Down;
		} else {
			gestrue_data_variable.status = Right;
		}
	}else{
		return(-1);
	}
	return(0);
}

//======================== main function ===========================================
int16_t gesture_sensor_initial(void)
{	
	uint8_t id;

    /* i2c device initial */
    if( i2c_device_initial() != 0){
        return(-1);
    }
    /* Read ID register and check against known values for APDS-9960 */
	if( read_register(APDS9960_ID, &id) != 0){
		return(-1);
	}
	if( id != APDS9960_ID_1 && id != APDS9960_ID_2 ){
        return(-1);
	}
	/* Set ENABLE register to 0 (disable all features) */
	if( set_mode(ALL, OFF) != 0){
		return(-1);
	}
	
	/* Set default values for ambient light and proximity registers */
	if( write_register(APDS9960_ATIME, DEFAULT_ATIME) != 0){
		return(-1);
	}
	if( write_register(APDS9960_WTIME, DEFAULT_WTIME) != 0){
		return(-1);
	}
	if( write_register(APDS9960_PPULSE, DEFAULT_PROX_PPULSE) != 0){
		return(-1);
	}
	if( write_register(APDS9960_POFFSET_UR, DEFAULT_POFFSET_UR) != 0){
		return(-1);
	}
	if( write_register(APDS9960_POFFSET_DL, DEFAULT_POFFSET_DL) != 0){
		return(-1);
	}
	if( write_register(APDS9960_CONFIG1, DEFAULT_CONFIG1) != 0){
		return(-1);
	}
	if( set_led_drive(DEFAULT_LDRIVE) != 0){
		return(-1);
	}
	if( set_proximity_gain(DEFAULT_PGAIN) != 0){
		return(-1);
	}
	if( set_ambient_light_gain(DEFAULT_AGAIN) != 0){
		return(-1);
	}
	if( set_prox_int_low_thresh(DEFAULT_PILT) != 0){
		return(-1);
	}
	if( set_prox_int_high_thresh(DEFAULT_PIHT) != 0){
		return(-1);
	}
	if( set_light_int_low_threshold(DEFAULT_AILT) != 0){
		return(-1);
	}
	if( set_light_int_high_threshold(DEFAULT_AIHT) != 0){
		return(-1);
	}
	if( write_register(APDS9960_PERS, DEFAULT_PERS) != 0){
		return(-1);
	}
	if( write_register(APDS9960_CONFIG2, DEFAULT_CONFIG2) != 0){
		return(-1);
	}
	if( write_register(APDS9960_CONFIG3, DEFAULT_CONFIG3) != 0){
		return(-1);
	}
	
	/* Set default values for gesture sense registers */
	if( set_gesture_enter_thresh(DEFAULT_GPENTH) != 0){
		return(-1);
	}
	if( set_gesture_exit_thresh(DEFAULT_GEXTH) != 0){
		return(-1);
	}
	if( write_register(APDS9960_GCONF1, DEFAULT_GCONF1) ){
		return(-1);
	}
	if( set_gesture_gain(DEFAULT_GGAIN) != 0){
		return(-1);
	}
	if( set_gesture_led_drive(DEFAULT_GLDRIVE) != 0){
		return(-1);
	}
	if( set_gesture_wait_time(DEFAULT_GWTIME) != 0){
		return(-1);
	}
	if( write_register(APDS9960_GOFFSET_U, DEFAULT_GOFFSET) != 0){
		return(-1);
	}
	if( write_register(APDS9960_GOFFSET_D, DEFAULT_GOFFSET) != 0){
		return(-1);
	}
	if( write_register(APDS9960_GOFFSET_L, DEFAULT_GOFFSET) != 0){
		return(-1);
	}
	if( write_register(APDS9960_GOFFSET_R, DEFAULT_GOFFSET) != 0){
		return(-1);
	}
	if( write_register(APDS9960_GPULSE, DEFAULT_GPULSE) != 0){
		return(-1);
	}
	if( write_register(APDS9960_GCONF3, DEFAULT_GCONF3) != 0){
		return(-1);
	}
	if( set_gesture_int_enable(DEFAULT_GIEN) != 0){
		return(-1);
	}
	
	//Clear Gestrue Variable
	memset(&gestrue_data_variable, 0x00, sizeof(struct gesture_data_t)); 
	
	//Initial INT GPIO
    gpio_unexport(INT_PIN_GPIO);
	if(gpio_export(INT_PIN_GPIO) != 0){
		return(-1);
	}
    if(gpio_direction(INT_PIN_GPIO, INPUT) != 0){
        return(-1);
    }
	// Start running the APDS-9960 gesture sensor engine
	if( enable_gesture_sensor(1) != 0){
		gpio_unexport(INT_PIN_GPIO);
		return(-1);
	}		
	return(0);
}

void gesture_sensor_close(void)
{
	//Clear Gestrue Variable
	memset(&gestrue_data_variable, 0x00, sizeof(struct gesture_data_t)); 
	
	set_gesture_int_enable(0);
	set_gesture_mode(0);
	set_mode(GESTURE, 0);
	
	gpio_unexport(INT_PIN_GPIO);
	close(i2c_fd);
}

int16_t gesture_sensor_is_ready(void)
{
	uint8_t val;
	
	//Check INT Pin Status
	if(gpio_read(INT_PIN_GPIO) != LOW){
		return(-1);
	}
	
	/* Read value from GSTATUS register */
	if( read_register(APDS9960_GSTATUS, &val) ) {
		return ERROR;
	}
	
	/* Shift and mask out GVALID bit */
	val &= APDS9960_GVALID;
	
	/* Return true/false based on GVALID bit */
	if( val == 1) {
		return(0);
	} else {
		return(-1);
	}
}

GESTURE_DIR gesture_sensor_get_direction_status(void)
{
	uint8_t  fifo_level = 0;
    uint8_t  bytes_read = 0;
    uint8_t  fifo_data[128];
    
    uint8_t  gstatus;
	int16_t	 res;
		
	uint32_t delay_time;
	
	uint16_t read_size,i;
	
	GESTURE_DIR type;
		
	/* Make sure that power and gesture is on and data is valid */
	res = get_mode();
	if(res == -1){
		return(None);
	}
		
	gstatus = (uint8_t)(res);
	if( !(gstatus & 0b01000001)){
		return(None);
	}
	
	while(1){
		/* Wait some time to collect next batch of FIFO data */
		delay_time = FIFO_PAUSE_TIME * 1000;
		usleep(delay_time);
		
		/* Get the contents of the STATUS register. Is data still valid? */
		if( read_register(APDS9960_GSTATUS, &gstatus) != 0){
			return(None);
		}
		
		if( (gstatus & APDS9960_GVALID) == APDS9960_GVALID ){
			
			/* Read the current FIFO level */
			if( read_register(APDS9960_GFLVL, &fifo_level) != 0){
				return(None);
			}
			
			/* If there's stuff in the FIFO, read it into our data block */
			if(fifo_level > 0){
				read_size = fifo_level * 4;
				if(read_data_block(APDS9960_GFIFO_U, fifo_data, read_size) != 0){
					return(None);
				}
				
				/* If at least 1 set of data, sort the data into U/D/L/R */
				if(read_size >= 4){
					for( i = 0; i < read_size; i += 4 ){
						gestrue_data_variable.value_array[gestrue_data_variable.array_size].up  	= fifo_data[i + 0];
						gestrue_data_variable.value_array[gestrue_data_variable.array_size].down  	= fifo_data[i + 1];
						gestrue_data_variable.value_array[gestrue_data_variable.array_size].left  	= fifo_data[i + 2];
						gestrue_data_variable.value_array[gestrue_data_variable.array_size].right  	= fifo_data[i + 3];
						gestrue_data_variable.array_size++;
					}					
				}	
				
				if(gesture_sensor_process_data() == 0){
					gesture_sensor_decode();
				}				
				gestrue_data_variable.array_size = 0;
			}
		}else{
			/* Wait some time to collect next batch of FIFO data */
			delay_time = FIFO_PAUSE_TIME * 1000;
			usleep(delay_time);
			
			gesture_sensor_decode();			
			type = gestrue_data_variable.status;
			
			//Clear Gestrue Variable
			memset(&gestrue_data_variable, 0x00, sizeof(struct gesture_data_t)); 
			gestrue_data_variable.status = None;
			
			return(type);
		}
	}
}

