#include "library.h"

struct ble_node *header = NULL;

int main(int argc, char *argv[])
{
	
	
	
	
#if 0
	struct ble_node *node = NULL;
	union color_t    color;
	uint8_t 		 cnt  = 0;
	uint8_t 		 buf[4];
	uint16_t 		 current_t_value = 0,current_h_value = 0, previous_t_value = 0;
	
	header = initial_ble_gatt();
	if(header == NULL){
		return(0);
	}
	printf("ble initial OK!!\n");
	sleep(1);
    while(1){
		node = header -> next;
		while(node != NULL){
			memset(buf, 0x00, sizeof(buf));
			read_ble_gatt_value(node, TEMPERATURE_HANDEL, buf);
			current_t_value = (int16_t)(buf[0]);
			
			memset(buf, 0x00, sizeof(buf));
			read_ble_gatt_value(node, HUMIDITY_HANDEL,    buf);
			current_h_value = (int16_t)(buf[0]);
			
			if(current_t_value != previous_t_value){
				previous_t_value = current_t_value;
				color = get_temperature_rgb_value(current_t_value);
				
				memset(buf, 0x00, sizeof(buf));
				buf[0] = color.member.r;
				buf[1] = color.member.g;
				buf[2] = color.member.b;
				buf[3] = 0xff;
				write_ble_gatt_value(node, LIGHT_HANDEL,    buf, 4);
			}
			printf("temperature=%d\thumidity=%d\n", current_t_value, current_h_value);
		    node = node -> next;
        }
		sleep(10);
	}
#endif

	GESTURE_DIR			   type;
 
 	if(gesture_sensor_initial() < 0){
		printf("Gesture Sensor Initial Fail!\n");
		return(0);
	}
     printf("start!!\n");
    while(1){
		
	    while(gesture_sensor_is_ready() < 0);
        type = gesture_sensor_get_direction_status();
	    switch(type){
			case Left:
				printf("DIR = Left\n");
				break;
			case Right:
				printf("DIR = Right\n");
				break;
			case Up:
				printf("DIR = Up\n");
				break;
			case Down:
				printf("DIR = Down\n");
				break;
			case Near:
				printf("DIR = Near\n");
				break;
			case Far	:
				printf("DIR = Far	\n");
				break;
				
			default:
				break;
		}
    }
  
    return(0);
}
