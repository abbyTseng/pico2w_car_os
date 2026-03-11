#include "app_blink.h"

#include <stdio.h>

#include "hal/hal_led.h"

// 引入 FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

// --- LED 閃爍 Task ---
void vAppBlinkTask(void *pvParameters)
{
    (void)pvParameters;  // 忽略未使用參數

    printf("[App Blink] Initializing LED...\n");
    const LedDevice *led = hal_led_get_default();
    if (led && led->init)
    {
        led->init();
    }

    // (已移除原先舊的 GPIO 22 按鈕邏輯，因為交由 app_button 處理了)

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
        // Day16 mission:💀 致命劇毒：對無效的記憶體位址進行寫入
        // volatile uint32_t *fatal_ptr = (volatile uint32_t *)0xDEADBEEF;
        //*fatal_ptr = 0xBAD;  // 保證死機！                  // 強制寫入，觸發 HardFault

        // 使用 vTaskDelayUntil 避免 Jitter
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}