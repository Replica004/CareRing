#ifndef FY_Lib_H
#define FY_Lib_H

/* Includes */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_efuse.h"
#include "esp_efuse_table.h" // 假设 MAC 地址字段定义在这个文件中

#include "gpio.h"
#include "nvs_ee.h"
#include "TCP_Client.h"

/* Defines */

/* Exported types ------------------------------------------------------------*/

/* Public function declarations */
bool Rec_Monitor_Ctrl(uint8_t *rx_buffer);
bool parse_config_string(const char *received_str,
                         char *out_ssid, size_t ssid_max_len,
                         char *out_password, size_t password_max_len,
                         char *out_ip, size_t ip_max_len,
                         char *out_port, size_t port_max_len);
void CMP_UserID_Password(const char *user_id, const char *password, 
                         const char *server_ip, const char *server_port);
void get_chip_id(void);

/* Global variables */

/* Exported types ------------------------------------------------------------*/





/* Public function declarations */


/* Global variables */
extern char Device_ID[];

#endif // FY_Lib_H
