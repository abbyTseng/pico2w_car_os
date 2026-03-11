#include "app_fsm.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "app_fsm_table.h"
#include "app_monitor.h"
#include "app_sync.h"  // 引入同步屏障
#include "queue.h"

#define FSM_QUEUE_LENGTH 10
#define FSM_QUEUE_ITEM_SIZE sizeof(fsm_event_t)

static QueueHandle_t xFsmQueue = NULL;
static fsm_state_t current_state = FSM_STATE_INIT;

bool app_fsm_send_event(fsm_event_t event)
{
    if (xFsmQueue == NULL) return false;

    // 嚴格規範：此 API 僅限 Task 層級呼叫。
    return (xQueueSend(xFsmQueue, &event, 0) == pdPASS);
}

fsm_state_t app_fsm_get_current_state(void) { return current_state; }

void vAppFsmTask(void *pvParameters)
{
    (void)pvParameters;
    fsm_event_t incoming_event;

    // 1. 建立 Event Queue
    xFsmQueue = xQueueCreate(FSM_QUEUE_LENGTH, FSM_QUEUE_ITEM_SIZE);
    configASSERT(xFsmQueue);

    printf("[FSM Task] Started. Current State: INIT. Waiting for Barrier...\n");

    // === 【新增的啟動同步屏障】 ===
    // 阻塞等待最多 3000ms，期間 CPU 負載為 0%
    if (app_sync_wait_for_all(3000))
    {
        // 屏障通過，自動推送 INIT_DONE 事件
        app_fsm_send_event(FSM_EVENT_INIT_DONE);
    }
    else
    {
        // 屏障失敗 (Timeout 或 Error)，強制進入 FAULT
        app_fsm_send_event(FSM_EVENT_ERROR);
    }
    // ============================

    // 2. O(1) 狀態機超級迴圈
    while (1)
    {
        // 等待事件到來 (無限期阻塞，釋放 CPU 給其他 Task)
        if (xQueueReceive(xFsmQueue, &incoming_event, pdMS_TO_TICKS(50)) == pdPASS)
        {
            // 安全防護：檢查 Event 是否越界
            if (incoming_event >= FSM_EVENT_MAX) continue;

            // === 核心查表邏輯 O(1) ===
            fsm_transition_t transition = g_fsm_table[current_state][incoming_event];
            fsm_state_t next_state = transition.next_state;

            // 如果有定義合法的狀態轉移
            if (next_state != 0 || transition.action != NULL)
            {
                // 執行轉移動作
                if (transition.action != NULL)
                {
                    transition.action();
                }

                // 狀態流轉
                if (next_state != current_state)
                {
                    printf("[FSM] State Transited: %d -> %d\n", current_state, next_state);
                    current_state = next_state;
                }
            }
            else
            {
                // 無效事件攔截
                printf("[FSM Warning] Invalid Event %d in State %d. Ignored.\n", incoming_event,
                       current_state);
            }
        }
        else
        {
            // Timeout 了，代表 50ms 內沒事件發生，系統閒置中。
            // 我們依然要舉起心跳，告訴 WDT "我是閒著，不是當機"
            app_monitor_report_heartbeat(HEARTBEAT_BIT_FSM);
        }
    }
}