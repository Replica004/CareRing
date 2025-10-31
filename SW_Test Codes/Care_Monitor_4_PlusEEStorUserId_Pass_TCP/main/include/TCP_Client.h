#ifndef __TCP_CLIENT_H
#define __TCP_CLIENT_H


#include "freertos/event_groups.h"
#include "freertos/semphr.h" // 新增: 用于互斥锁
#include "esp_wifi.h"
#include "esp_event.h"


// Required for socket communication
#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h" // For socket functions
#include <arpa/inet.h>   // For inet_addr


/* Defines */
#define MY_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define MY_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define MY_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY

#if CONFIG_ESP_WPA3_SAE_PWE_HUNT_AND_PECK
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HUNT_AND_PECK
#define EXAMPLE_H2E_IDENTIFIER ""
#elif CONFIG_ESP_WPA3_SAE_PWE_HASH_TO_ELEMENT
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_HASH_TO_ELEMENT
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#elif CONFIG_ESP_WPA3_SAE_PWE_BOTH
#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER CONFIG_ESP_WIFI_PW_ID
#endif
#if CONFIG_ESP_WIFI_AUTH_OPEN
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#elif CONFIG_ESP_WIFI_AUTH_WEP
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WEP
#elif CONFIG_ESP_WIFI_AUTH_WPA_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA_WPA2_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA_WPA2_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WPA2_WPA3_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_WPA3_PSK
#elif CONFIG_ESP_WIFI_AUTH_WAPI_PSK
#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WAPI_PSK
#endif

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void wifi_init_sta(void);

// 新增：声明主动发送数据的子程序
int tcp_send_data(uint8_t *buffer, size_t len);
/*******************************************************************************
 * Global variables
 ******************************************************************************/
extern char ble_ssid[32];                       
extern char ble_password[64]; 
extern char SERVER_IP[16];                       
extern char SERVER_PORT[8]; 

// 新增：声明全局的 TCP socket 句柄和互斥锁
extern int g_tcp_socket;
extern SemaphoreHandle_t g_tcp_socket_mutex;
extern bool connect_status; // 声明 connect_status，因为你在 TCP_Client.c 中定义了它

#endif /* __TCP_CLIENT_H */

