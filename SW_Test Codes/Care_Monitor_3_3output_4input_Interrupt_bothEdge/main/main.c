#include "Project_Lib.h"

void app_main(void)
{
    gpio_init();

    while (1)
    {
        
        led_off();
        vTaskDelay(500 / portTICK_PERIOD_MS);
        
        led_on();
        vTaskDelay(500 / portTICK_PERIOD_MS);       
    }
}

