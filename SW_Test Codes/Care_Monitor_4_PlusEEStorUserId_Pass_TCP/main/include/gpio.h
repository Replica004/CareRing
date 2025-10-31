/*
 * SPDX-FileCopyrightText: 2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */
#ifndef LED_H
#define LED_H

/* Includes */
#include "led_strip.h"

/* Defines */
#define GPIO_OUTPUT_IO_1    CONFIG_GPIO_OUTPUT_1
#define GPIO_OUTPUT_IO_2    CONFIG_GPIO_OUTPUT_2
#define GPIO_OUTPUT_IO_3    CONFIG_GPIO_OUTPUT_3
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<GPIO_OUTPUT_IO_1) | (1ULL<<GPIO_OUTPUT_IO_2) | (1ULL<<GPIO_OUTPUT_IO_3))

#define GPIO_INPUT_IO_0    CONFIG_GPIO_INPUT_0
#define GPIO_INPUT_IO_1    CONFIG_GPIO_INPUT_1
#define GPIO_INPUT_IO_2    CONFIG_GPIO_INPUT_2
#define GPIO_INPUT_IO_3    CONFIG_GPIO_INPUT_3
#define GPIO_INPUT_PIN_SEL  ((1ULL<<GPIO_INPUT_IO_0) | (1ULL<<GPIO_INPUT_IO_1) | (1ULL<<GPIO_INPUT_IO_2) | (1ULL<<GPIO_INPUT_IO_3))

#define GPIO_OUTPUT_BLINK   CONFIG_BLINK_GPIO

/* Exported types ------------------------------------------------------------*/


/* Public function declarations */
uint8_t get_led_state(void);
void led_on(void);
void led_off(void);
void gpio_on(uint8_t num);
void gpio_off(uint8_t num);
void gpio_all_off(void);
void gpio_init(void);

/* Global variables */
extern bool Total_Switch_EN;

#endif // LED_H
