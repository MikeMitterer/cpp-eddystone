//
// Created by Mike Mitterer on 15.01.18.
//

#ifndef NODEMCUESP32_GPIO_TASK_H
#define NODEMCUESP32_GPIO_TASK_H

#include <cstdint>
#include <driver/gpio.h>

const uint32_t GPIO_SSR_ON = 0;
const uint32_t GPIO_SSR_OFF = 1;

const uint32_t LED_ON = 1;
const uint32_t LED_OFF = 0;

const gpio_num_t internalLED = GPIO_NUM_2;

void init_gpio(gpio_num_t pin, gpio_mode_t mode);

#endif //NODEMCUESP32_GPIO_TASK_H
