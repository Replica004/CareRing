#include "Project_Lib.h"

void app_main(void)
{
    gpio_init();

    while (1)
    {
        gpio_off(1);
        led_off();
        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_on(1);
        led_on();
        vTaskDelay(500 / portTICK_PERIOD_MS);       
    }
}

