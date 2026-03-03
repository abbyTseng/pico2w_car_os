#include "app_blink.h"

#include <stdio.h>

#include "hal/hal_gpio.h"
#include "hal/hal_led.h"

// 引入 FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

// Callback 函式
static void on_button_press(uint32_t pin, uint32_t event)
{
    // 嚴格標準：明確告訴編譯器這兩個參數目前不用，消除 unused-parameter 警告
    (void)pin;
    (void)event;
}

// --- LED 閃爍 Task ---
void vAppBlinkTask(void *pvParameters)
{
    (void)pvParameters;  // 忽略未使用參數

    printf("[App Blink] Initializing LED & GPIO...\n");
    const LedDevice *led = hal_led_get_default();
    if (led && led->init)
    {
        led->init();
    }

    hal_gpio_init_input(22);
    hal_gpio_set_callback(on_button_press);

    // 絕對週期設定
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(500);  // 500ms 週期

    while (1)
    {
        if (led && led->toggle)
        {
            led->toggle();
        }

        printf("[Task LED] Toggled. Executing on Core: %d\n", portGET_CORE_ID());

        // 使用 vTaskDelayUntil 避免 Jitter
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}