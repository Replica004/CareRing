#ifndef __NVS_EE_H
#define __NVS_EE_H

/* Includes */
#include "TCP_Client.h"

/* Defines */
#define EEPROM_Flag 0x5A

/* Exported types ------------------------------------------------------------*/
typedef struct
{
	 uint8_t		Flag;
     uint8_t		Wifi_Flag;
	 uint8_t		UserName[32];
	 uint8_t		Password[64];
	 uint8_t		Server_ip[16];
	 uint8_t		Server_port[6];
     uint16_t		CheckSum;

}Idt_EE_Config_T;

static const Idt_EE_Config_T Config_Default =
{
	.Flag = EEPROM_Flag,
	.Wifi_Flag = 0,
	.UserName = MY_ESP_WIFI_SSID,
	.Password = MY_ESP_WIFI_PASS,
	.Server_ip = "192.168.18.4",
	.Server_port = "55001",
};

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void nvs_int(void);
void Read_Config_Data(void);
void Write_Config(void);

/*******************************************************************************
 * Global variables
 ******************************************************************************/
extern Idt_EE_Config_T EE_Config_Data;

#endif /* __NVS_EE_H */
