#ifndef _BLE_GATT_
#define _BLE_GATT_

#include <stdint.h>

#define MAC_ADDR_SIZE_MAX    18
 
#define TEMPERATURE_HANDEL	 0
#define HUMIDITY_HANDEL		 1
#define LIGHT_HANDEL		 2

struct ble_node
{
	char             mac[MAC_ADDR_SIZE_MAX];
	int16_t		 	 handle[3];
	struct ble_node	 *next;
};

struct ble_node *initial_ble_gatt    (void);
int16_t 		 read_ble_gatt_value (struct ble_node*,uint8_t, uint8_t[]);
int16_t 		 write_ble_gatt_value(struct ble_node*,uint8_t, uint8_t[], uint8_t);

#endif
