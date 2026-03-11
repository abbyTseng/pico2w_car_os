/**
 * @file hal_led.c
 * @brief LED HAL implementation for Pico 2 W (Wireless).
 * Uses CYW43 architecture instead of standard GPIO.
 */

#include "hal_led.h"

#include <stdbool.h>

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h"

// --- 私有變數 ---
static bool is_initialized = false;
static bool current_logical_state = false;  // 用軟體紀錄目前是亮還是暗

// --- 私有實作函數 (不公開) ---

static common_status_t pico_w_led_init(void)
{
    common_status_t status = COMMON_OK;

    if (is_initialized == false)
    {  // MISRA C: 明確比較，不使用 !is_initialized
        // 初始化無線晶片
        if (cyw43_arch_init() != 0)
        {
            status = COMMON_ERR;
        }
        else
        {
            is_initialized = true;
            status = COMMON_OK;
        }
    }
    else
    {
        // 已經初始化過，視為成功
        status = COMMON_OK;
    }
    return status;  // 單一出口
}

static void pico_w_led_set_state(LedState state)
{
    if (is_initialized == true)
    {
        bool is_on = (state == LED_ON);

        // 更新硬體
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, is_on);

        // 更新軟體紀錄
        current_logical_state = is_on;
    }
}

static void pico_w_led_toggle(void)
{
    if (is_initialized == true)
    {
        // 反轉狀態
        current_logical_state = !current_logical_state;

        // 遞迴呼叫 set_state 以保持一致性
        pico_w_led_set_state(current_logical_state ? LED_ON : LED_OFF);
    }
}

// --- 綁定介面 ---
static const LedDevice pico_w_led_driver = {
    .init = &pico_w_led_init,  // MISRA C: 函數指標建議加上 &
    .set_state = &pico_w_led_set_state,
    .toggle = &pico_w_led_toggle};

// --- 公開函數 ---
const LedDevice *hal_led_get_default(void) { return &pico_w_led_driver; }
