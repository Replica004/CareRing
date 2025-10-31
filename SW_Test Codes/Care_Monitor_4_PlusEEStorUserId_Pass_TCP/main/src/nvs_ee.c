/* Includes */
#include "Project_Lib.h"


static const char *TAG = "NVS_STORAGE";

Idt_EE_Config_T EE_Config_Data;

void save_config(Idt_EE_Config_T *config_data) {
    nvs_handle_t my_handle;
    esp_err_t err;

    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return;
    }

    err = nvs_set_blob(my_handle, "my_config", config_data, sizeof(Idt_EE_Config_T));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to write config BLOB (%s)!", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Config BLOB written successfully.");
    }

    nvs_close(my_handle);
}

uint16_t Get_Checksum(void)
{
	uint8_t i, len, *src;
	uint16_t checksum=0;

    src = (uint8_t *)&EE_Config_Data.Flag;
    len = sizeof(EE_Config_Data)-2;

	for(i=0; i<len; i++)
	{
		checksum += *src++;
	}

	return checksum;

}

void Write_Config(void)
{
	uint16_t checksum;

	checksum = Get_Checksum();
	EE_Config_Data.CheckSum = checksum;

    save_config(&EE_Config_Data);

    ESP_LOGI(TAG, "EEPROM data written successfully with checksum: %u", checksum);

}

void Write_Default_Config(void)
{
    EE_Config_Data =  Config_Default;   
// write data to nvm
    Write_Config();
}

void load_config(Idt_EE_Config_T *config_data) {
    nvs_handle_t my_handle;
    esp_err_t err;
    size_t required_size = sizeof(Idt_EE_Config_T);

    err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return;
    }

    err = nvs_get_blob(my_handle, "my_config", config_data, &required_size);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read config BLOB (%s)!", esp_err_to_name(err));
        // 如果是 ESP_ERR_NVS_NOT_FOUND，可能需要初始化一个默认配置
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGW(TAG, "Config not found in NVS, initializing with default values.");
            // 这里可以设置 config_data 为默认值
        }
    } else {
        ESP_LOGI(TAG, "Config BLOB read successfully, size: %zu bytes.", required_size);
            ESP_LOGI(TAG, "--- Idt_EE_Config_T Data ---");
            ESP_LOGI(TAG, "Flag: %u", EE_Config_Data.Flag);
            ESP_LOGI(TAG, "Wifi_Flag: %u", EE_Config_Data.Wifi_Flag);
            // 打印字符串，确保以 null 结尾，这里假设 UserName 和 Password 已经是有效的C字符串
            ESP_LOGI(TAG, "UserName: %s", EE_Config_Data.UserName);
            ESP_LOGI(TAG, "Password: %s", EE_Config_Data.Password);
            ESP_LOGI(TAG, "CheckSum: %u", EE_Config_Data.CheckSum);
            ESP_LOGI(TAG, "----------------------------");
    }

    nvs_close(my_handle);
}

void Read_Config_Data(void)
{
	uint16_t Checksum;

	load_config(&EE_Config_Data);

	Checksum = Get_Checksum();

	if(EE_Config_Data.Flag != EEPROM_Flag || EE_Config_Data.CheckSum != Checksum)
	{
		Write_Default_Config();
	}else
	{
		//nothing to do
	}

    strcpy((char *)ble_ssid, (char *)EE_Config_Data.UserName);
    strcpy((char *)ble_password, (char *)EE_Config_Data.Password);
    strcpy((char *)SERVER_IP, (char *)EE_Config_Data.Server_ip);
    strcpy((char *)SERVER_PORT, (char *)EE_Config_Data.Server_port);
}

void nvs_int(void)
{
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        // Retry nvs_flash_init
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );
}
