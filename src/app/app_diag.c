/* src/app/app_diag.c */
#include "app_diag.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "app_monitor.h"
#include "app_sync.h"
#include "hal/hal_storage.h"
#include "semphr.h"
#include "task.h"

static dtc_record_t g_dtc_db[10];
static bool g_is_dirty = false;
static SemaphoreHandle_t g_diag_mutex = NULL;

common_status_t app_diag_init(void)
{
    g_diag_mutex = xSemaphoreCreateMutex();
    configASSERT(g_diag_mutex != NULL);

    // 從 Flash 讀取現有的 DTC (如果有)
    hal_storage_read_file("dtc.bin", (uint8_t *)g_dtc_db, sizeof(g_dtc_db));
    return COMMON_OK;
}

void app_diag_report_event(uint16_t dtc_id, bool failed)
{
    if (xSemaphoreTake(g_diag_mutex, pdMS_TO_TICKS(10)) != pdTRUE) return;

    for (int i = 0; i < 10; i++)
    {
        if (g_dtc_db[i].dtc_id == dtc_id || g_dtc_db[i].dtc_id == 0)
        {
            g_dtc_db[i].dtc_id = dtc_id;
            // 實作簡易 Debounce：只有連續錯誤才升級為 Confirmed
            if (failed)
            {
                g_dtc_db[i].occurrence_count++;
                if (g_dtc_db[i].occurrence_count >= 1 &&
                    !(g_dtc_db[i].status & DTC_STATUS_BIT_CDTC))
                {
                    g_dtc_db[i].status |= DTC_STATUS_BIT_CDTC;
                    g_is_dirty = true;
                }
            }
            else
            {
                // 如果測試通過，可以實作老化機制，或僅重置當前週期的 Failed Bit
                g_dtc_db[i].status &= ~DTC_STATUS_BIT_TF;
            }
            break;
        }
    }
    xSemaphoreGive(g_diag_mutex);
}

// 【新增】將 DTC 寫入 LittleFS
void app_diag_sync_to_storage(void)
{
    common_status_t status =
        hal_storage_write_file("dtc.bin", (uint8_t *)g_dtc_db, sizeof(g_dtc_db));
    if (status == COMMON_OK)
    {
        g_is_dirty = false;
        printf("💾 [Diag] DTC Database synchronized to Flash successfully.\n");
    }
    else
    {
        printf("❌ [Diag] DTC Database sync failed!\n");
    }
}

void vAppDiagTask(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    // 喚醒週期為 100ms，與看門狗節奏一致
    const TickType_t xFrequency = pdMS_TO_TICKS(100);
    uint32_t sync_timer_ms = 0;  // 本地計時器

    printf("[Diag] Task Started on Core %d\n", portGET_CORE_ID());

    while (1)
    {
        // 1. 每 100ms 報平安
        app_monitor_report_heartbeat(HEARTBEAT_BIT_DIAG);

        // 2. 累加本地計時器
        sync_timer_ms += 100;

        // 3. 滿 30 秒且有髒資料時寫入 Flash
        if (g_is_dirty && (sync_timer_ms >= 30000))
        {
            if (xSemaphoreTake(g_diag_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
            {
                app_diag_sync_to_storage();
                sync_timer_ms = 0;  // 寫入成功後重置
                xSemaphoreGive(g_diag_mutex);
            }
        }

        // 4. 精準睡 100ms
        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}