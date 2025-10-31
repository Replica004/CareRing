/* Includes */
#include "Project_Lib.h"

static const char *TAG = "GPIO_Monitor";

#ifdef CONFIG_EXAMPLE_BLINK_LED_STRIP
static led_strip_handle_t led_strip;
#endif

/* Public variables */
uint8_t led_state = 0; // 0: off, 1: on



/* Public functions */

#ifdef CONFIG_EXAMPLE_BLINK_LED_STRIP
void led_on(void)
{
    if(led_state == 0)
    {
        led_state = 1;
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        led_strip_set_pixel(led_strip, 0, 0, 8, 0);//red 16,green 16,blue 16
    }else if(led_state == 1)
    {
        led_state = 2;
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        led_strip_set_pixel(led_strip, 0, 0, 0, 8);//red 16,green 16,blue 16
    }else if( led_state == 2)
    {
        led_state = 0;
        /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
        led_strip_set_pixel(led_strip, 0, 8, 0, 0);//red 16,green 16,blue 16
    }
    /* Refresh the strip to send data */
    led_strip_refresh(led_strip);
}

void led_off(void)
{
    /* Set all LED off to clear all pixels */
    led_strip_clear(led_strip);
}

void led_init(void)
{
    // ESP_LOGI(TAG, "example configured to blink addressable led!");
    /* LED strip initialization with the GPIO and pixels number*/
    led_strip_config_t strip_config = {
        .strip_gpio_num = GPIO_OUTPUT_BLINK,
        .max_leds = 1, // at least one LED on board
    };
#if CONFIG_EXAMPLE_BLINK_LED_STRIP_BACKEND_RMT
    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000, // 10MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
#elif CONFIG_EXAMPLE_BLINK_LED_STRIP_BACKEND_SPI
    led_strip_spi_config_t spi_config = {
        .spi_bus = SPI2_HOST,
        .flags.with_dma = true,
    };
    ESP_ERROR_CHECK(
        led_strip_new_spi_device(&strip_config, &spi_config, &led_strip));
#else
#error "unsupported LED strip backend"
#endif
    /* Set all LED off to clear all pixels */
    led_off();
}

#elif CONFIG_EXAMPLE_BLINK_LED_GPIO

void led_on(void)
{
    gpio_set_level(GPIO_OUTPUT_BLINK, false);
}

void led_off(void)
{
    gpio_set_level(GPIO_OUTPUT_BLINK, true);
}

#else
#error "unsupported LED type"
#endif

void gpio_on(uint8_t num)
{
    if(num == 0)
    {
        gpio_set_level(GPIO_OUTPUT_IO_1, false);
    }
    else if(num == 1)
    {
        gpio_set_level(GPIO_OUTPUT_IO_2, false);
    }
    else if(num == 2)
    {
        gpio_set_level(GPIO_OUTPUT_IO_3, false);
    }
}

void gpio_off(uint8_t num)
{
    if(num == 0)
    {
        gpio_set_level(GPIO_OUTPUT_IO_1, true);
    }
    else if(num == 1)
    {
        gpio_set_level(GPIO_OUTPUT_IO_2, true);
    }
    else if(num == 2)
    {
        gpio_set_level(GPIO_OUTPUT_IO_3, true);
    }
}

void gpio_all_off(void)
{

    gpio_set_level(GPIO_OUTPUT_IO_1, false);
    gpio_set_level(GPIO_OUTPUT_IO_2, false);
    gpio_set_level(GPIO_OUTPUT_IO_3, false);

}

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t)arg;

    if (gpio_num == GPIO_INPUT_IO_0){

        ESP_DRAM_LOGI(TAG, "数据中断 input0 触发! ");

        if(gpio_get_level(gpio_num) == 0){
            gpio_off(0);
        }
        else{
            gpio_on(0);
        }
    }else if (gpio_num == GPIO_INPUT_IO_1){

        ESP_DRAM_LOGI(TAG, "数据中断 input1 触发! ");

        if(gpio_get_level(gpio_num) == 0){
            gpio_off(1);
        }
        else{
            gpio_on(1);
        }
    }else if (gpio_num == GPIO_INPUT_IO_2){

        ESP_DRAM_LOGI(TAG, "数据中断 input2 触发! ");

        if(gpio_get_level(gpio_num) == 0){
            gpio_off(2);
        }
        else{
            gpio_on(2);
        }
    }else if (gpio_num == GPIO_INPUT_IO_3){

        ESP_DRAM_LOGI(TAG, "数据中断 input3 触发! ");

        if(gpio_get_level(gpio_num) == 0){
            gpio_off(2);
        }
        else{
            gpio_on(2);
        }
    }
}

void gpio_init(void)
{
    led_init();

    gpio_config_t output_conf = {
        .pin_bit_mask = GPIO_OUTPUT_PIN_SEL,        // 选择引脚
        .mode = GPIO_MODE_OUTPUT,                   // 输出模式
        .intr_type = GPIO_INTR_DISABLE,             // 禁用中断
        .pull_up_en = GPIO_PULLUP_DISABLE,          // 禁用上拉（防悬空）
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    gpio_config(&output_conf);

    gpio_all_off();

    gpio_config_t input_conf = {
    .pin_bit_mask = GPIO_INPUT_PIN_SEL,        // 选择引脚
    .mode = GPIO_MODE_INPUT,                  // 输入模式
    .intr_type = GPIO_INTR_ANYEDGE,           // 上下降沿,中断
    .pull_up_en = GPIO_PULLUP_DISABLE,         // 禁用上拉（防悬空）
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
};
    gpio_config(&input_conf);

        //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);

    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(GPIO_INPUT_IO_0, gpio_isr_handler, (void*) GPIO_INPUT_IO_0);
    gpio_isr_handler_add(GPIO_INPUT_IO_1, gpio_isr_handler, (void*) GPIO_INPUT_IO_1);
    gpio_isr_handler_add(GPIO_INPUT_IO_2, gpio_isr_handler, (void*) GPIO_INPUT_IO_2);
    gpio_isr_handler_add(GPIO_INPUT_IO_3, gpio_isr_handler, (void*) GPIO_INPUT_IO_3);

}
