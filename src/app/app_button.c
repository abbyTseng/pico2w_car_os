/**
 * @file app_button.c
 * @brief Application layer for Button handling.
 */

#include "app_button.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "hal/hal_gpio.h"
#include "task.h"

// 假設你的杜邦線測試使用 GPIO 15 (可依據你的硬體修改)
#define APP_BUTTON_PIN 15
// 定義去彈跳時間 (50ms)
#define DEBOUNCE_TIME_MS 50

// 儲存 Task Handle，讓 ISR 知道要把訊號發給誰
static TaskHandle_t xButtonTaskHandle = NULL;

/**
 * @brief 在硬體中斷 (ISR Context) 內執行的回呼函式
 * @warning 絕對不可在此處呼叫任何阻塞 API (如 printf, vTaskDelay)
 */
static void _button_isr(uint32_t pin, uint32_t events)
{
    (void)pin;
    (void)events;

    // 必須初始化為 pdFALSE
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (xButtonTaskHandle != NULL)
    {
        // 1. 發送 Direct Task Notification 喚醒 Task
        vTaskNotifyGiveFromISR(xButtonTaskHandle, &xHigherPriorityTaskWoken);

        // 2. 如果被喚醒的 Task 優先權比當前被中斷的 Task 高，則觸發一次 Context Switch
        // 這是確保「即時性 (Real-time)」的關鍵！
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}

void app_button_init(void)
{
    hal_gpio_init_input(APP_BUTTON_PIN);
    hal_gpio_set_isr_callback(_button_isr);
}

void vAppButtonTask(void *pvParameters)
{
    (void)pvParameters;

    // 1. 將自己的 Task Handle 存起來，準備接收 ISR 通知
    xButtonTaskHandle = xTaskGetCurrentTaskHandle();

    TickType_t xLastWakeTime = 0;
    uint32_t press_count = 0;

    // 2. 初始化硬體與 ISR 註冊
    app_button_init();
    printf("[APP_BUTTON] Task Started. Listening on GPIO %d...\n", APP_BUTTON_PIN);
    for (;;)
    {
        // 3. 進入阻塞狀態，等待 ISR 通知。
        // pdTRUE 表示醒來後將通知計數器清零 (類似 Binary Semaphore)
        // portMAX_DELAY 表示死等，不消耗任何 CPU 時間
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // --- 醒來了，現在是安全的 Thread Context ---

        TickType_t xCurrentTime = xTaskGetTickCount();

        // 4. 軟體去彈跳 (Timestamp Filtering)
        // 只有與上一次有效按壓時間相隔大於 DEBOUNCE_TIME_MS，才視為有效
        if ((xCurrentTime - xLastWakeTime) > pdMS_TO_TICKS(DEBOUNCE_TIME_MS))
        {
            press_count++;
            printf("[APP_BUTTON] Valid Press Detected! Count: %lu\n", press_count);

            // 更新最後一次有效觸發的時間戳
            xLastWakeTime = xCurrentTime;
            // 【Test 2 注入點】模擬繁重任務卡住 CPU 500ms
            // printf("[APP_BUTTON] Task is BUSY for 500ms...\n");
            // vTaskDelay(pdMS_TO_TICKS(500));
            // printf("[APP_BUTTON] Task is READY again.\n");
        }
        else
        {
            // 收到雜訊 (Bouncing)，直接忽略。
            // 故意拔插杜邦線時，這裡會攔截到成百上千次的雜訊。
            // 你可以解開下一行的註解來觀察雜訊有多誇張：
            // printf("[APP_BUTTON] Bouncing ignored! (Delta: %lu ms)\n",
            //       (uint32_t)pdTICKS_TO_MS(xCurrentTime - xLastWakeTime));
        }
    }
}