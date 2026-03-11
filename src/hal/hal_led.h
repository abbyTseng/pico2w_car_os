/**
 * @file hal_led.h
 * @brief LED abstraction. App uses this; no direct SDK in app.
 */

#ifndef HAL_LED_H
#define HAL_LED_H

#include <stdbool.h>

#include "common_status.h"
#include "common_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // 定義 LED 狀態
    typedef enum
    {
        LED_OFF = 0U,  // MISRA C: Enum 值建議顯式定義，並加上 U (Unsigned)
        LED_ON = 1U
    } LedState;

    // 【核心架構】LED 裝置介面 (V-Table)
    // 任何想要當 LED 的硬體，都要實作這三個函數
    typedef struct
    {
        common_status_t (*init)(void);
        void (*set_state)(LedState state);
        void (*toggle)(void);
    } LedDevice;

    /**
     * Toggle board LED state.
     */
    // 取得預設的 LED 裝置實體
    const LedDevice *hal_led_get_default(void);

#ifdef __cplusplus
}
#endif

#endif /* HAL_LED_H */
