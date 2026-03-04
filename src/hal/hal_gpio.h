#ifndef HAL_GPIO_H
#define HAL_GPIO_H

#include "common_types.h"

// 初始化 GPIO 為輸入模式 (含上拉電阻)
void hal_gpio_init_input(uint32_t pin);

// 註冊 Callback (觀察者)
void hal_gpio_set_isr_callback(hal_gpio_isr_callback_t cb);

#endif