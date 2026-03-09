#include "app_sync.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "event_groups.h"
#include "task.h"

/* 宣告 Event Group Handle (隱藏在 .c 檔中，實踐封裝) */
static EventGroupHandle_t xSyncEventGroup = NULL;

void app_sync_init(void)
{
    if (xSyncEventGroup == NULL)
    {
        xSyncEventGroup = xEventGroupCreate();
        configASSERT(xSyncEventGroup);  // 車規防呆：記憶體不足直接停機
        printf("[App Sync] Boot Barrier Initialized.\n");
    }
}

void app_sync_report_ready(uint32_t ready_bit)
{
    if (xSyncEventGroup != NULL)
    {
        // 設定對應的 Ready Bit
        xEventGroupSetBits(xSyncEventGroup, ready_bit);
        printf("[App Sync] Module Ready Bit Set: 0x%04X\n", (unsigned int)ready_bit);
    }
}

void app_sync_report_error(uint32_t error_bit)
{
    if (xSyncEventGroup != NULL)
    {
        // 設定對應的 Error Bit (觸發 Fast-Fail)
        xEventGroupSetBits(xSyncEventGroup, error_bit);
        printf("[App Sync] Module ERROR Bit Set: 0x%04X\n", (unsigned int)error_bit);
    }
}

bool app_sync_wait_for_all(uint32_t timeout_ms)
{
    if (xSyncEventGroup == NULL) return false;

    TickType_t xTicksToWait = pdMS_TO_TICKS(timeout_ms);
    TimeOut_t xTimeOut;
    vTaskSetTimeOutState(&xTimeOut);

    /* * 架構技巧：我們需要同時監聽「全部 Ready (AND)」或「任一 Error (OR)」。
     * 因此我們讓 xWaitForAllBits = pdFALSE (任一 Bit 觸發即喚醒)，並用 while 迴圈自行判斷。
     */
    while (xTaskCheckForTimeOut(&xTimeOut, &xTicksToWait) == pdFALSE)
    {
        EventBits_t uxBits =
            xEventGroupWaitBits(xSyncEventGroup, SYNC_ALL_READY_MASK | SYNC_ANY_ERROR_MASK,
                                pdFALSE,      // xClearOnExit: 先不清除，由我們手動決定
                                pdFALSE,      // xWaitForAllBits: 只要有任何一個 Bit 被 Set 就喚醒
                                xTicksToWait  // 剩餘的 Block 時間
            );

        // 1. 檢查是否有任何錯誤發生 (Fast-Fail 邏輯)
        if ((uxBits & SYNC_ANY_ERROR_MASK) != 0U)
        {
            printf("[App Sync] BARRIER FAILED: Error detected (Bits: 0x%04X)\n",
                   (unsigned int)uxBits);
            xEventGroupClearBits(xSyncEventGroup, SYNC_ALL_READY_MASK | SYNC_ANY_ERROR_MASK);
            return false;
        }

        // 2. 檢查是否「所有」模組都已就緒
        if ((uxBits & SYNC_ALL_READY_MASK) == SYNC_ALL_READY_MASK)
        {
            printf("[App Sync] BARRIER PASSED: All modules ready!\n");
            // 越過屏障後，清除所有 Bits 以防影響後續重啟
            xEventGroupClearBits(xSyncEventGroup, SYNC_ALL_READY_MASK | SYNC_ANY_ERROR_MASK);
            return true;
        }

        // 如果只是部分 Ready，迴圈會繼續 Block 等待剩餘的時間...
    }

    printf("[App Sync] BARRIER TIMEOUT: Wait exceeded %u ms\n", (unsigned int)timeout_ms);
    return false;
}