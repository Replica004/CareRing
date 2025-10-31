/* Includes */
#include "Project_Lib.h"

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi station";
static const char *TCP_TAG = "tcp_client"; // Tag for TCP client logs

static int s_retry_num = 0;
bool connect_status = false;

char ble_ssid[32];                       
char ble_password[64]; 

char SERVER_IP[16] = "192.168.18.4" ;                       
char SERVER_PORT[8] = "55001" ; 

// --- TCP Client Specific Defines ---
//#define SERVER_IP   "192.168.2.18"    //"76.66.101.143"
//#define SERVER_PORT 55001
#define KEEPALIVE_IDLE_SECONDS 5    // Keepalive idle time in seconds
#define KEEPALIVE_INTERVAL_SECONDS 1 // Keepalive interval in seconds
#define KEEPALIVE_COUNT 3           // Number of keepalive probes

// --- 定义全局的 TCP socket 句柄和互斥锁 ---
int g_tcp_socket = -1; // 全局变量定义，初始值为 -1 表示未连接
SemaphoreHandle_t g_tcp_socket_mutex = NULL; // 互斥锁定义，初始为 NULL

static void tcp_client_task(void *pvParameters); // Forward declaration

static void event_handler(void* arg, esp_event_base_t event_base,
                                 int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < MY_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        // --- IMPORTANT: Start TCP client task after getting IP ---
        // 确保只创建一次任务，如果已经创建了，则不重复创建
        static bool tcp_task_created = false;
        if (!tcp_task_created) {
            xTaskCreate(tcp_client_task, "tcp_client_task", 4096, NULL, 5, NULL);
            tcp_task_created = true;
        }
    }
}

wifi_config_t wifi_config = {
    .sta = {
        /* Authmode threshold resets to WPA2 as default if password matches WPA2 standards (password len => 8).
        * If you want to connect the device to deprecated WEP/WPA networks, Please set the threshold value
        * to WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK and set the password with length and format matching to
        * WIFI_AUTH_WEP/WIFI_AUTH_WPA_PSK standards.
        */
        .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
        .sae_pwe_h2e = ESP_WIFI_SAE_MODE,
        .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER,
    },
};

void Replace_SSID_Password(void)
{
    // 复制 SSID
    strncpy((char *)wifi_config.sta.ssid, (char *)ble_ssid, sizeof(wifi_config.sta.ssid) - 1);
    wifi_config.sta.ssid[sizeof(wifi_config.sta.ssid) - 1] = '\0';

    // 复制密码
    strncpy((char *)wifi_config.sta.password, (char *)ble_password, sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.password[sizeof(wifi_config.sta.password) - 1] = '\0';
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    
    Replace_SSID_Password();

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 wifi_config.sta.ssid, wifi_config.sta.password);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 wifi_config.sta.ssid, wifi_config.sta.password);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }
}


// --- 主动向 TCP 服务器发送数据的子程序 ---
/**
 * @brief 主动向 TCP 服务器发送数据
 *
 * @param buffer 要发送的数据缓冲区指针
 * @param len 要发送的数据长度
 * @return int 实际发送的字节数，-1 表示发送失败或连接未建立
 */
int tcp_send_data(uint8_t *buffer, size_t len)
{
    if (buffer == NULL || len == 0) {
        ESP_LOGW(TCP_TAG, "Attempted to send null buffer or zero length data.");
        return 0; // 成功发送0字节
    }

    int bytes_sent = -1;

    // 尝试获取互斥锁，保护对 g_tcp_socket 的访问和发送操作
    // 设置一个超时时间，防止长时间阻塞
    if (g_tcp_socket_mutex != NULL && xSemaphoreTake(g_tcp_socket_mutex, pdMS_TO_TICKS(500)) == pdTRUE) {
        if (g_tcp_socket != -1) {
            // 连接已建立，可以发送数据
            bytes_sent = send(g_tcp_socket, buffer, len, 0);
            if (bytes_sent < 0) {
                ESP_LOGE(TCP_TAG, "Error sending data: errno %d", errno);
            } else if ((size_t)bytes_sent < len) {
                ESP_LOGW(TCP_TAG, "Only sent %d of %d bytes", bytes_sent, len);
            } else {
                ESP_LOGI(TCP_TAG, "Successfully sent %d bytes.", bytes_sent);
            }
        } else {
            ESP_LOGW(TCP_TAG, "TCP socket not connected. Cannot send data.");
        }
        xSemaphoreGive(g_tcp_socket_mutex); // 释放互斥锁
    } else {
        ESP_LOGW(TCP_TAG, "Failed to acquire TCP socket mutex within timeout. Cannot send data.");
    }

    return bytes_sent;
}

// --- New TCP Client Task ---
static void tcp_client_task(void *pvParameters)
{
    uint8_t rx_buffer[1461];
    //char host_ip[] = SERVER_IP;
    int addr_family = 0;
    int ip_protocol = 0;

    struct sockaddr_in dest_addr;
    dest_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(atoi(SERVER_PORT));
    addr_family = AF_INET;
    ip_protocol = IPPROTO_IP;

    int sock = -1; // Socket descriptor

    // 在任务开始时创建互斥锁，确保只创建一次
    if (g_tcp_socket_mutex == NULL) {
        g_tcp_socket_mutex = xSemaphoreCreateMutex();
        if (g_tcp_socket_mutex == NULL) {
            ESP_LOGE(TCP_TAG, "Failed to create TCP socket mutex!");
            vTaskDelete(NULL); // 无法创建互斥锁，任务无法继续
        }
    }

    while (1) {
        sock = socket(addr_family, SOCK_STREAM, ip_protocol);
        if (sock < 0) {
            ESP_LOGE(TCP_TAG, "Unable to create socket: errno %d", errno);
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        // Set TCP Keepalive options (optional, but good for robustness)
        int keepAlive = 1;
        int keepIdle = KEEPALIVE_IDLE_SECONDS;
        int keepInterval = KEEPALIVE_INTERVAL_SECONDS;
        int keepCount = KEEPALIVE_COUNT;
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &keepAlive, sizeof(keepAlive));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &keepIdle, sizeof(keepIdle));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &keepInterval, sizeof(keepInterval));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &keepCount, sizeof(keepCount));

        ESP_LOGI(TCP_TAG, "Socket created, connecting to %s:%d", SERVER_IP, atoi(SERVER_PORT));

        int err = connect(sock, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in));
        if (err != 0) {
            ESP_LOGE(TCP_TAG, "Socket unable to connect: errno %d", errno);
            close(sock);
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        ESP_LOGI(TCP_TAG, "Successfully connected to server");

                // --- 连接成功时，更新全局 socket 句柄 ---
        if (xSemaphoreTake(g_tcp_socket_mutex, portMAX_DELAY) == pdTRUE) {
            g_tcp_socket = sock; // 将当前 socket 句柄赋值给全局变量
            xSemaphoreGive(g_tcp_socket_mutex);
        } else {
            ESP_LOGE(TCP_TAG, "Failed to acquire mutex to update global socket. Closing connection.");
            close(sock);
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }
        //connect_status = true; // 更新连接状态
        // --- Communication Loop ---
        while (1) {
            // Example: Send data to the server
            if(connect_status == false)
            {
                connect_status = true;

                // Define a buffer large enough to hold the combined message
                char combined_buffer[50]; // Choose a size appropriate for your longest possible message

                // Use snprintf to safely format the combined string
                // snprintf is safer than sprintf because it prevents buffer overflows
                int len = snprintf(combined_buffer, sizeof(combined_buffer), 
                                "Care Monitor Connected,Device_ID:%s\n", Device_ID);

                // Check for snprintf errors (optional but good practice)
                if (len < 0) {
                    ESP_LOGE(TCP_TAG, "snprintf failed.");
                    // Handle error
                } else {
                    // Send the combined buffer
                    int written = send(sock, combined_buffer, len, 0);
                    if (written < 0) {
                        ESP_LOGE(TCP_TAG, "Error occurred during sending: errno %d", errno);
                        break; // Break to reconnect
                    }
                    ESP_LOGI(TCP_TAG, "Sent %d bytes: %s", written, combined_buffer);
                }               
            }

            // Example: Receive data from the server
            int rx_len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
            if (rx_len < 0) {
                ESP_LOGE(TCP_TAG, "Error occurred during receiving: errno %d", errno);
                break; // Break to reconnect
            } else if (rx_len == 0) {
                ESP_LOGW(TCP_TAG, "Connection closed by remote host");
                break; // Break to reconnect
            } else {

                if(rx_len > 1460)       //limit max data length =1460
                {
                    rx_len = 1460;
                }

                rx_buffer[rx_len] = 0; // Null-terminate received data
                ESP_LOGI(TCP_TAG, "Received %d bytes: %s", rx_len, rx_buffer);

                if (Rec_Monitor_Ctrl(rx_buffer)) {

                    const char *tx_buffer = "CMD_Rec_OK\n";
                    int len = strlen(tx_buffer);
                    int written = send(sock, tx_buffer, len, 0);
                    if (written < 0) {
                        ESP_LOGE(TCP_TAG, "Error occurred during sending: errno %d", errno);
                        break; // Break to reconnect
                    }
              
                } else {

                    // Handle unknown command
                    const char *tx_buffer = "CMD_Rec_Error\n";
                    int len = strlen(tx_buffer);
                    int written = send(sock, tx_buffer, len, 0);
                    if (written < 0) {
                        ESP_LOGE(TCP_TAG, "Error occurred during sending: errno %d", errno);
                        break; // Break to reconnect
                    }
                }
            }
            vTaskDelay(pdMS_TO_TICKS(100)); // Wait 5 seconds before sending next data
        }

                // --- 连接断开或错误，清理全局 socket 句柄 ---
        if (xSemaphoreTake(g_tcp_socket_mutex, portMAX_DELAY) == pdTRUE) {
            g_tcp_socket = -1; // 重置全局句柄
            xSemaphoreGive(g_tcp_socket_mutex);
        } else {
            ESP_LOGE(TCP_TAG, "Failed to acquire mutex during cleanup. Global socket might be stale.");
        }
        connect_status = false; // 更新连接状态

        if (sock != -1) {
            ESP_LOGE(TCP_TAG, "Shutting down socket and restarting...");
            shutdown(sock, 0); // Shut down send side
            close(sock);
        }
        vTaskDelay(pdMS_TO_TICKS(2000)); // Wait before attempting to reconnect
    }
    vTaskDelete(NULL); // Should ideally not be reached
}

