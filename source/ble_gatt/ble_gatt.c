#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "ble_gatt.h"

#define CHARACTERISTICS_LIST_SIZE_MAX    10
#define BUF_SIZE_MAX                	 256
#define COMMAND_SIZE_MAX         	 	 256
#define VALUE_HEX_ARRAY_SIZE_MAX		 32
#define ERROR_MESSAGE_SIZE               13
#define CHARACTERISTICS_VALUE_SIZE_MAX	 1024

struct gatt_characteristics_t
{
    unsigned int 	handle;
    unsigned int  	properties;
    unsigned int 	value_handle;
    unsigned int 	uuid;
};

//======================== sub function ===========================================
static
uint8_t ascii_to_hex(char text[])
{
	uint8_t byte_value,temp;
	
	byte_value = 0x00;	
	if(isdigit(text[0])){
		temp = text[0] - '0';
	}else{
		temp  = text[0] - 'a';
		temp += 0x0A;
	}
	byte_value = temp << 4;
	
	if(isdigit(text[1])){
		temp = text[1] - '0';
	}else{
		temp  = text[1] - 'a';
		temp += 0x0A;
	}
	byte_value |= temp;
	
	return(byte_value);
}

static
uint8_t hex_string_to_hex_array(uint8_t tgt[], char src[])
{
	uint8_t  size = 0;

	while(*src != '\0'){
		if(isblank(*src)){
			src++;			
		}else{
			tgt[size] = ascii_to_hex(src);
			src += 2;
			size+= 1;	
		}
	}
	return(size);
}

static
void hex_array_to_hex_string(char tgt[], uint8_t src[], uint8_t size)
{
	const char hex_ascii_tb[]=
	{
		'0',
		'1',
		'2',
		'3',
		'4',
		'5',
		'6',
		'7',
		'8',
		'9',
		'a',
		'b',
		'c',
		'd',
		'e',
		'f'
	};

	uint8_t temp;

	while(size){
		 temp   = *src;
		 //H-Byte
		*tgt    = hex_ascii_tb[temp >> 4];  
		 tgt   += 1;
		 //L-Byte 
		*tgt    = hex_ascii_tb[temp & 0x0F];  
		 tgt   += 1;
		 
		 src   += 1;
		 size  -= 1;
	}
	*tgt = '\0';
}

//======================== main function ===========================================
int16_t read_ble_gatt_value(struct ble_node *node, uint8_t handle_number, uint8_t result_array[])
{
    char command_string[COMMAND_SIZE_MAX] ="\0";
    char read_buffer[BUF_SIZE_MAX]        ="\0";
    char *result_text					  = NULL;
    int16_t array_size;
	FILE *popen_fp;
	
//check handlke Value
//=========================================================================			
	if(node -> handle[handle_number] == 0xFFFF){
		return(-1);
	}

//Get BLE Characteristics Value
//=========================================================================		
	command_string[0] = '\0';
	sprintf(command_string,
	   	    "gatttool -t random -b %s --char-read --handle=%d 2>&1",
		     node -> mac,
		     node -> handle[handle_number]);
    popen_fp = popen(command_string,"r");
  	if(popen_fp == NULL){
		return(-1);
   	}
	read_buffer[0] = '\0';
 	fgets(read_buffer, BUF_SIZE_MAX, popen_fp);	
	pclose(popen_fp);

// Check Result
//=========================================================================			
     memcpy(&command_string[0], &read_buffer[0], ERROR_MESSAGE_SIZE);
     command_string[ERROR_MESSAGE_SIZE] = '\0';
     if(strcmp(command_string , "connect error") == 0){
		return(-1);
     } 
//Format Staring	
//=========================================================================	
	result_text = strchr(&read_buffer[0], ':');
	if(result_text == NULL){
		return(-1);
	}
	result_text++;
	
	array_size  = -1;
	array_size  = hex_string_to_hex_array(result_array, result_text);
	return(array_size);
}


int16_t write_ble_gatt_value(struct ble_node *node, uint8_t handle_number, uint8_t data_array[], uint8_t data_array_size)
{
	char command_string[COMMAND_SIZE_MAX] ="\0";
    char read_buffer[BUF_SIZE_MAX]        ="\0";
    char *result_text					  = NULL;
	int16_t array_size;
	FILE *popen_fp;
	
//check handlke Value
//=========================================================================			
	if(node -> handle[handle_number] == 0xFFFF){
		return(-1);
	}	
//Format Staring	
//=========================================================================
	hex_array_to_hex_string(read_buffer, data_array, data_array_size);

//Set BLE Characteristics Value
//=========================================================================		
	command_string[0] = '\0';
	sprintf(command_string,
	   	    "gatttool -t random -b %s --char-write-cmd --handle=%d --value=%s 2>&1",
		     node -> mac,
		     node -> handle[handle_number],
             read_buffer);
   	popen_fp = popen(command_string,"r");
   	if(popen_fp == NULL){
		return(-1);
   	}   	
   	read_buffer[0] = '\0';
	fgets(read_buffer, BUF_SIZE_MAX, popen_fp);
	pclose(popen_fp);

// Check Result
//=========================================================================			
    if(strcmp(read_buffer , "Characteristic value was written successfully") != 0){
        return(-1);
    } 
	return(0);	
}


struct ble_node *initial_ble_gatt(void)
{
    char command_string[COMMAND_SIZE_MAX]                      	     ="\0";
    char mac_address[MAC_ADDR_SIZE_MAX]                 	   	     ="\0";
    char read_buffer[BUF_SIZE_MAX]                             		 ="\0";
    char result_buffer[BUF_SIZE_MAX * CHARACTERISTICS_LIST_SIZE_MAX ]="\0";
    char *text_search,*text_current;
	FILE *popen_fp, *fp;
	
	struct ble_node 			 *header,*node;
	struct gatt_characteristics_t characteristics_member;
// HCI Enable	
//=========================================================================	
	popen_fp = popen("hciconfig hci0 up","r");
	if(popen_fp == NULL){
		printf("Open HCI Fail!\n");
		return(NULL);
	}
	result_buffer[0] = '\0'; 
    read_buffer[0]   = '\0';
	while(fgets(read_buffer, BUF_SIZE_MAX, popen_fp)){
		strcat(result_buffer, read_buffer);
	}
	pclose(popen_fp);

// Get Mac Address	
//=========================================================================		
	//Open File
	read_buffer[0] = '\0';
	if(getcwd(read_buffer, sizeof(read_buffer)) == NULL){
		printf("Get Current DIR Name Fail!\n");
	}
	strcat(read_buffer, "configuration/ble_mac");
	fp = fopen(read_buffer,"r");
    if(fp == NULL){
        printf("Open %s flie Error!\n",read_buffer);
        return(NULL); 
    }
 
 // Initial 'Header' Link List   
 //=========================================================================	   
    //Set Header Node
    header = (struct ble_node*)malloc(sizeof(struct ble_node));
    if(header == NULL){
        fclose(fp);
        printf("Memory Fail!\n");
        return(NULL);
    }
    memset(header, 0x00, sizeof(struct ble_node));
 
// Get All 'Characteristics'   
//=========================================================================		
	//Get 'BLE MAC List"
    while(fgets(mac_address, MAC_ADDR_SIZE_MAX, fp) != NULL){
        if(strlen(mac_address) != MAC_ADDR_SIZE_MAX - 1){
            break;
        }
        //Get BLE Characteristics
	   	command_string[0] = '\0';
	   	sprintf(command_string,
				"gatttool -t random -b %s --characteristics 2>&1",
				 mac_address);
      	
        popen_fp = popen(command_string,"r");
       	if(popen_fp == NULL){
	   	 	pclose(popen_fp);
		 	continue;
	   	}
// Get Result	
//=========================================================================			
	   	// Use 'GATTtool'
	   	result_buffer[0] = '\0'; 
       	read_buffer[0]   = '\0';
		while(fgets(read_buffer, BUF_SIZE_MAX, popen_fp)){
	   		strcat(result_buffer, read_buffer);
	   	}
	   	pclose(popen_fp);
// Check Result
//=========================================================================			
        memcpy(command_string, result_buffer, ERROR_MESSAGE_SIZE);
        command_string[ERROR_MESSAGE_SIZE] = '\0';
        if(strcmp(command_string , "connect error") == 0){
			printf("BLE Link Fail!\n");
		    continue;
        } 
	   	
//=========================================================================		   
	   	//Set 'Node' Value
	   	node = (struct ble_node*)malloc(sizeof(struct ble_node));
	   	if(node == NULL){
		 	continue;
		}
		node -> handle[0]   = -1;
		node -> handle[1]   = -1;
		node -> handle[2]   = -1;
	   	node -> mac[0]      = '\0';
	   	node -> next        = NULL;
		strcat(node -> mac, mac_address);

	   	text_current = &result_buffer[0];
	   	do{
	    	text_search = strchr(text_current, '\n');
		  	if(text_search != NULL){
		    	*text_search = '\0';
             	text_search++;
			 	//format
			 	sscanf(text_current,
                       "handle = 0x%x, char properties = 0x%x, char value handle = 0x%x, uuid = %x-",
                       &characteristics_member.handle,
                       &characteristics_member.properties,
                       &characteristics_member.value_handle,
                       &characteristics_member.uuid);
			 
			 	switch(characteristics_member.uuid){
			   		case 0xFFFD:
						node->handle[TEMPERATURE_HANDEL] = characteristics_member.value_handle;
						break;
					case 0xFFFE:
						node->handle[HUMIDITY_HANDEL]    = characteristics_member.value_handle;
						break;
					case 0xFFF1:
						node->handle[LIGHT_HANDEL]       = characteristics_member.value_handle;
						break;
					default:
						break;
		     	}
		  	}
		  	text_current = text_search;
		}while(text_current != NULL);
		
		//Link List
		node   -> next  = header -> next;
    	header -> next  = node;
	}
//=========================================================================		 
	fclose(fp);
	return(header);
} 
