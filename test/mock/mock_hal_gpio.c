#include "hal_gpio.h"

// 這是給 App 層測試用的「假 HAL」
// 當 app_blink.c 呼叫這些函式時，什麼事都不會發生 (Stub)

void hal_gpio_init_input(uint32_t pin)
{
    (void)pin;  // 假裝初始化了
}

void hal_gpio_init_output(uint32_t pin)
{
    (void)pin;  // 假裝初始化了
}

void hal_gpio_set_isr_callback(hal_gpio_isr_callback_t callback)
{
    (void)callback;  // 假裝註冊了，實際上不存
}
