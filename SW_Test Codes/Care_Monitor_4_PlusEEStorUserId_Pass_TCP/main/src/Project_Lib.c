/* Includes */
#include "Project_Lib.h"

static const char *TCP_TAG = "tcp_client"; // Tag for TCP client logs
static const char *TAG = "NVS_STORAGE";

uint8_t mac_addr[6];
char Device_ID[] = "123456789012";

static const char *TAGMAC = "MAC";

/**
 * @brief 将字节数组转换为十六进制字符串，使用查表法
 *
 * @param src_data 输入的字节数组
 * @param data_len 输入数组的长度
 * @param dest_str 输出的十六进制字符串，其大小必须至少是 (data_len * 2 + 1)
 */
void hex_to_string(const unsigned char *src_data, size_t data_len, char *dest_str) {
    if (src_data == NULL || dest_str == NULL) {
        return;
    }

    // 十六进制字符表
    const char hex_chars[] = "0123456789abcdef";

    for (size_t i = 0; i < data_len; i++) {
        // 取当前字节的高4位
        unsigned char high_nibble = (src_data[i] >> 4) & 0x0F;
        // 取当前字节的低4位
        unsigned char low_nibble = src_data[i] & 0x0F;

        // 根据高4位和低4位在字符表中查找对应的字符
        dest_str[i * 2] = hex_chars[high_nibble];
        dest_str[i * 2 + 1] = hex_chars[low_nibble];
    }

    // 在字符串末尾添加终止符 '\0'
    dest_str[data_len * 2] = '\0';
}

// 专门用于获取和打印芯片 ID 的函数
void get_chip_id(void)
{
    
    // 使用 esp_efuse_read_field_blob 函数读取 MAC 地址
    // ESP_EFUSE_MAC 应该是一个指向 esp_efuse_desc_t 数组的指针
    esp_err_t ret = esp_efuse_read_field_blob(ESP_EFUSE_MAC, mac_addr, sizeof(mac_addr) * 8);

    hex_to_string(mac_addr, sizeof(mac_addr), Device_ID);

    if (ret == ESP_OK) {
        ESP_LOGI(TAGMAC, "Chip MAC Address: %02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    } else {
        ESP_LOGE(TAGMAC, "Failed to read MAC address from eFuse, error: %s", esp_err_to_name(ret));
    }
}

bool Rec_Monitor_Ctrl(uint8_t *rx_buffer)
{
    bool status = true;
    uint8_t index=0, i;

    for(i = 0; i < 4; i++)
    {
        if(rx_buffer[index+i] != 'c')     {
            break;
        }
    }

    if(i !=4)
    {
        status = false;
    }
    else{

        index +=4;

        //command control
        ESP_LOGI(TCP_TAG, "Command: '%s'", rx_buffer);
        
    }
    
    return status;
}

//when receive user id, password, ip address and port from bluetooth or uart
void CMP_UserID_Password(const char *user_id, const char *password, const char *server_ip, const char *server_port)
{
    // 比较输入的 user_id 和 password 是否与存储的值匹配
    // --- 比较并更新 UserName ---
    // 确保比较的是同一个结构体实例，这里假定EE_Config_Data是主配置
    if (strcmp((char*)EE_Config_Data.UserName, user_id) != 0) {
        // 用户名不相等，进行更新
        EE_Config_Data.Wifi_Flag = 1; // 标记配置需要保存

        size_t user_id_len = strlen(user_id);
        // !! 关键修正：防止缓冲区溢出 !!
        // 确保拷贝长度不超过目标缓冲区大小 - 1 (为null终止符留位)
        size_t copy_len_user = user_id_len;
        if (copy_len_user >= sizeof(EE_Config_Data.UserName)) {
            copy_len_user = sizeof(EE_Config_Data.UserName) - 1;
            ESP_LOGW(TAG, "User ID too long, truncating.");
        }
        strncpy((char*)EE_Config_Data.UserName, user_id, copy_len_user);
        EE_Config_Data.UserName[copy_len_user] = '\0'; // 确保null终止

        ESP_LOGI(TAG, "UserName changed to: '%s'", EE_Config_Data.UserName);
    } else {
        ESP_LOGI(TAG, "UserName is unchanged: '%s'", EE_Config_Data.UserName); // 保持一致性
    }

    // --- 比较并更新 Password ---
    if (strcmp((char*)EE_Config_Data.Password, password) != 0) { // 同样比较 EE_Config_Data
        // 密码不相等，进行更新
        EE_Config_Data.Wifi_Flag = 1; // 标记配置需要保存

        size_t password_len = strlen(password);
        // !! 关键修正：防止缓冲区溢出 !!
        // 确保拷贝长度不超过目标缓冲区大小 - 1 (为null终止符留位)
        size_t copy_len_pass = password_len;
        if (copy_len_pass >= sizeof(EE_Config_Data.Password)) {
            copy_len_pass = sizeof(EE_Config_Data.Password) - 1;
            ESP_LOGW(TAG, "Password too long, truncating.");
        }
        strncpy((char*)EE_Config_Data.Password, password, copy_len_pass);
        EE_Config_Data.Password[copy_len_pass] = '\0'; // 确保null终止

        ESP_LOGI(TAG, "Password changed to: '%s'", EE_Config_Data.Password);

    } else {
        ESP_LOGI(TAG, "Password is unchanged: '%s'", EE_Config_Data.Password); // 保持一致性
    }

    // --- 比较并更新 Server_IP ---
    if (strcmp((char*)EE_Config_Data.Server_ip, server_ip) != 0) { // 同样比较 EE_Config_Data
        // 密码不相等，进行更新
        EE_Config_Data.Wifi_Flag = 1; // 标记配置需要保存

        size_t server_ip_len = strlen(server_ip);
        // !! 关键修正：防止缓冲区溢出 !!
        // 确保拷贝长度不超过目标缓冲区大小 - 1 (为null终止符留位)
        size_t copy_len_pass = server_ip_len;
        if (copy_len_pass >= sizeof(EE_Config_Data.Server_ip)) {
            copy_len_pass = sizeof(EE_Config_Data.Server_ip) - 1;
            ESP_LOGW(TAG, "Server_ip too long, truncating.");
        }
        strncpy((char*)EE_Config_Data.Server_ip, server_ip, copy_len_pass);
        EE_Config_Data.Server_ip[copy_len_pass] = '\0'; // 确保null终止

        ESP_LOGI(TAG, "Server_IP changed to: '%s'", EE_Config_Data.Server_ip);

    } else {
        ESP_LOGI(TAG, "Server_IP is unchanged: '%s'", EE_Config_Data.Server_ip); // 保持一致性
    }

    // --- 比较并更新 Server_Port ---
    if (strcmp((char*)EE_Config_Data.Server_port, server_port) != 0) { // 同样比较 EE_Config_Data
        // 密码不相等，进行更新
        EE_Config_Data.Wifi_Flag = 1; // 标记配置需要保存

        size_t server_port_len = strlen(server_port);
        // !! 关键修正：防止缓冲区溢出 !!
        // 确保拷贝长度不超过目标缓冲区大小 - 1 (为null终止符留位)
        size_t copy_len_pass = server_port_len;
        if (copy_len_pass >= sizeof(EE_Config_Data.Server_port)) {
            copy_len_pass = sizeof(EE_Config_Data.Server_port) - 1;
            ESP_LOGW(TAG, "Server_port too long, truncating.");
        }
        strncpy((char*)EE_Config_Data.Server_port, server_port, copy_len_pass);
        EE_Config_Data.Server_port[copy_len_pass] = '\0'; // 确保null终止

        ESP_LOGI(TAG, "Server_Port changed to: '%s'", EE_Config_Data.Server_port);

    } else {
        ESP_LOGI(TAG, "Server_Port is unchanged: '%s'", EE_Config_Data.Server_port); // 保持一致性
    }

    // --- 保存逻辑 ---
    if(EE_Config_Data.Wifi_Flag == 1){ // 如果 Flag 被设置为1（表示有更改）
        EE_Config_Data.Wifi_Flag = 2; // 更改为2，表示已处理过或正在保存
                                          // 您可能需要根据您的具体状态机来定义这个值
                                          // 比如，2可能表示“待保存但尚未完成写入”或者“已保存”
        // !! 关键修正：传入结构体指针 !!
        Write_Config(); 

        // Replace with your own SSID and Password
    	strcpy((char *)ble_ssid, (char *)EE_Config_Data.UserName);
    	strcpy((char *)ble_password, (char *)EE_Config_Data.Password);
        strcpy((char *)SERVER_IP, (char *)EE_Config_Data.Server_ip);
    	strcpy((char *)SERVER_PORT, (char *)EE_Config_Data.Server_port);

        ESP_LOGI(TAG, "Configuration updated and marked for saving, it needs restart.");
    }
}

/**
 * @brief 解析从Flutter应用接收到的配置字符串。
 * 字符串格式预期为: "SSID:<ssid>\nPASS:<password>\nIP:<ip>\nPORT:<port>"
 * 函数会提取ssid, password, ip, port到各自的缓冲区中。
 *
 * @param received_str 接收到的完整配置字符串。
 * @param out_ssid 用于存储提取的SSID的缓冲区。
 * @param ssid_max_len out_ssid缓冲区的最大长度（包括null终止符）。
 * @param out_password 用于存储提取的密码的缓冲区。
 * @param password_max_len out_password缓冲区的最大长度（包括null终止符）。
 * @param out_ip 用于存储提取的IP地址的缓冲区。
 * @param ip_max_len out_ip缓冲区的最大长度（包括null终止符）。
 * @param out_port 用于存储提取的端口号的缓冲区。
 * @param port_max_len out_port缓冲区的最大长度（包括null终止符）。
 *
 * @return true 如果所有字段都成功提取并复制，false 如果解析失败或缓冲区溢出。
 */
bool parse_config_string(const char *received_str,
                         char *out_ssid, size_t ssid_max_len,
                         char *out_password, size_t password_max_len,
                         char *out_ip, size_t ip_max_len,
                         char *out_port, size_t port_max_len)
{
    // 创建一个可修改的字符串副本，因为strtok会修改原始字符串
    char *str_copy = strdup(received_str);
    if (str_copy == NULL) {
        // 内存分配失败
        return false;
    }

    char *token;
    char *rest = str_copy; // strtok_r 的上下文指针

    // 1. 提取 SSID
    token = strtok_r(rest, "\n", &rest);
    if (token == NULL || !strstr(token, "+++SSID:")) {
        free(str_copy);
        return false;
    }
    strncpy(out_ssid, token + strlen("+++SSID:"), ssid_max_len - 1);
    out_ssid[ssid_max_len - 1] = '\0'; // 确保null终止

    // 2. 提取 Password
    token = strtok_r(rest, "\n", &rest);
    if (token == NULL || !strstr(token, "PASS:")) {
        free(str_copy);
        return false;
    }
    strncpy(out_password, token + strlen("PASS:"), password_max_len - 1);
    out_password[password_max_len - 1] = '\0'; // 确保null终止

    // 3. 提取 IP
    token = strtok_r(rest, "\n", &rest);
    if (token == NULL || !strstr(token, "IP:")) {
        free(str_copy);
        return false;
    }
    strncpy(out_ip, token + strlen("IP:"), ip_max_len - 1);
    out_ip[ip_max_len - 1] = '\0'; // 确保null终止

    // 4. 提取 Port
    token = strtok_r(rest, "\n", &rest);
    if (token == NULL || !strstr(token, "PORT:")) {
        free(str_copy);
        return false;
    }
    strncpy(out_port, token + strlen("PORT:"), port_max_len - 1);
    out_port[port_max_len - 1] = '\0'; // 确保null终止

    free(str_copy); // 释放副本内存
    return true;
}
