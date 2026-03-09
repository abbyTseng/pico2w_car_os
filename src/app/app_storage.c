#include "app_storage.h"

#include <stdint.h>
#include <stdio.h>

#include "app/app_fsm.h"
#include "app_sync.h"
#include "common/common_status.h"
#include "hal/hal_storage.h"

// 引入 FreeRTOS
#include "FreeRTOS.h"
#include "task.h"

#define BOOT_COUNT_FILE "sys_boot.bin"

common_status_t app_storage_log_boot(void)
{
    uint32_t count = 0;

    // 1. 嘗試讀取現有的開機次數
    (void)hal_storage_read_file(BOOT_COUNT_FILE, (uint8_t *)&count, sizeof(count));

    count++;
    printf("[App Storage] Current Boot Count: %u\n", (unsigned int)count);

    // 2. 寫回新的計數
    return hal_storage_write_file(BOOT_COUNT_FILE, (uint8_t *)&count, sizeof(count));
}

// --- 背景監控 Task ---
void vMonitorTask(void *pvParameters)
{
    (void)pvParameters;

    app_storage_log_boot();

    // === 模擬各模組初始化所需的時間差異 ===
    vTaskDelay(pdMS_TO_TICKS(500));
    app_sync_report_ready(SYNC_BIT_DISPLAY_READY);

    vTaskDelay(pdMS_TO_TICKS(800));
    app_sync_report_ready(SYNC_BIT_STORAGE_READY);

    vTaskDelay(pdMS_TO_TICKS(400));
    app_sync_report_ready(SYNC_BIT_SENSOR_READY);
    // ===================================

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(5000);

    while (1)
    {
        // 進入穩態，不再亂發 Event 干擾 FSM
        printf("[Task Monitor] System OK. Core: %d\n", portGET_CORE_ID());
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}