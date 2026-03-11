#include "app_monitor.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "event_groups.h"
#include "hal/hal_wdt.h"
#include "task.h"

// 隱藏的 Event Group
static EventGroupHandle_t xHeartbeatGroup = NULL;

// 軟體 WDT 參數
#define MONITOR_CYCLE_MS 100
#define WINDOW_MIN_MS 50  // 提早餵狗防護窗 (小於此時間視為異常)

void app_monitor_init(void)
{
    if (xHeartbeatGroup == NULL)
    {
        xHeartbeatGroup = xEventGroupCreate();
        configASSERT(xHeartbeatGroup);
    }
}

void app_monitor_report_heartbeat(uint32_t task_bit)
{
    if (xHeartbeatGroup != NULL)
    {
        xEventGroupSetBits(xHeartbeatGroup, task_bit);
    }
}

// 可獨立測試的純邏輯
bool app_monitor_evaluate_system_state(uint32_t current_bits, uint32_t elapsed_ms)
{
    // 1. 檢查是否所有 Task 都有回報 (避免 Starvation/Deadlock)
    if ((current_bits & HEARTBEAT_MASK_ALL) != HEARTBEAT_MASK_ALL)
    {
        return false;
    }

    // 2. Window Watchdog: 檢查是否過早回報 (防範死迴圈)
    if (elapsed_ms < WINDOW_MIN_MS)
    {
        return false;
    }

    return true;
}

// Monitor Task 本體
void vAppMonitorTask(void *pvParameters)
{
    (void)pvParameters;

    printf("[Task Monitor] Boot Grace Period: 3000ms...\n");
    // 容忍開機階段 CYW43 或高優先級 Lab Tasks 的 CPU 佔用
    vTaskDelay(pdMS_TO_TICKS(3000));

    app_monitor_init();
    hal_wdt_init(1500);  // 硬體 WDT 設為 1.5 秒
    printf("[Task Monitor] Hardware WDT Armed!\n");

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(MONITOR_CYCLE_MS);

    while (1)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        // 讀取並同時清除所有 Bit (Atomic 操作，非常安全)
        EventBits_t uxBits = xEventGroupClearBits(xHeartbeatGroup, HEARTBEAT_MASK_ALL);

        // 評估系統健康度
        if (app_monitor_evaluate_system_state(uxBits, MONITOR_CYCLE_MS))
        {
            hal_wdt_kick();  // 健康，餵狗
            // printf("[Task Monitor] WDT Kicked. System Healthy.\n");
        }
        else
        {
            // 異常！記錄 Error (未來可以寫入 Flash)，並拒絕餵狗
            printf("\n[FATAL] Task Deadlock/Starvation Detected! Bits: 0x%02X\n",
                   (unsigned int)uxBits);
            printf("[FATAL] Monitor stopped kicking WDT. Awaiting System Reset...\n\n");

            // 將自己掛起，坐等硬體 WDT 1.5s 後將系統 Reset
            vTaskSuspend(NULL);
        }
    }
}