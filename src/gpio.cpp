//
// Created by Mike Mitterer on 15.01.18.
//

/* Copyright (c) 2017 pcbreflux. All Rights Reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>. *
 */

#include "gpio.h"

#include <cstdio>
#include <esp_system.h>

/* LED state */
extern int global_counter;

void init_gpio(gpio_num_t pin,gpio_mode_t mode) {
    gpio_config_t io_conf{0};

    io_conf.pin_bit_mask = static_cast<uint64_t>(1 << pin);
    io_conf.intr_type = static_cast<gpio_int_type_t>(GPIO_PIN_INTR_DISABLE);

    io_conf.mode = mode;

    if (GPIO_SSR_ON == 1) { // high side, pull down
        io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;

    } else { // low side, pull up
        io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
        io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;

    }
    gpio_config(&io_conf);

}

