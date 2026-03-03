#include "app_storage.h"

#include <stdint.h>
#include <stdio.h>

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
    (void)pvParameters;  // 忽略未使用參數

    // 開機時記錄一次
    app_storage_log_boot();

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1000);  // 1000ms 週期

    while (1)
    {
        // portGET_CORE_ID() 可以抓出現在在哪個核心執行
        printf("[Task Monitor] System OK. Executing on Core: %d\n", portGET_CORE_ID());
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}