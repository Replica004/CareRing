/* Includes */
#include "Project_Lib.h"


#ifdef CONFIG_EXAMPLE_BLINK_LED_STRIP
static led_strip_handle_t led_strip;
#endif

/* Public variables */

/* Public functions */

#ifdef CONFIG_EXAMPLE_BLINK_LED_STRIP
void led_on(void)
{
    /* Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color */
    led_strip_set_pixel(led_strip, 0, 10, 0, 0);

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
    if(num == 1)
    {
        gpio_set_level(GPIO_OUTPUT_IO_1, false);
    }
    else if(num == 2)
    {
        gpio_set_level(GPIO_OUTPUT_IO_2, false);
    }
    else if(num == 3)
    {
        gpio_set_level(GPIO_OUTPUT_IO_3, false);
    }
}

void gpio_off(uint8_t num)
{
    if(num == 1)
    {
        gpio_set_level(GPIO_OUTPUT_IO_1, true);
    }
    else if(num == 2)
    {
        gpio_set_level(GPIO_OUTPUT_IO_2, true);
    }
    else if(num == 3)
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

void gpio_init(void)
{
    led_init();

    // ESP_LOGI(TAG, "example configured to blink gpio led!");
    gpio_reset_pin(GPIO_OUTPUT_IO_1);
    gpio_reset_pin(GPIO_OUTPUT_IO_2);
    gpio_reset_pin(GPIO_OUTPUT_IO_3);

    /* Set the GPIO as a push/pull output */

    gpio_set_direction(GPIO_OUTPUT_IO_1, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_OUTPUT_IO_2, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_OUTPUT_IO_3, GPIO_MODE_OUTPUT);

    gpio_all_off();


}
