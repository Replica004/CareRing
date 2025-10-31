#include "Project_Lib.h"

void app_main(void)
{
    gpio_init();
    nvs_int();
    Read_Config_Data();
    // 再获取芯片 ID
    get_chip_id();

    wifi_init_sta();

    while (1)
    {
        
        led_off();
        vTaskDelay(500 / portTICK_PERIOD_MS);
        
        led_on();
        vTaskDelay(500 / portTICK_PERIOD_MS);       
    }
}

