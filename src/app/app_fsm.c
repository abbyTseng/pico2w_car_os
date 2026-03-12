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

// 【新增】將查表邏輯獨立出來，方便單元測試呼叫
static void fsm_process_event(fsm_event_t incoming_event)
{
    if (incoming_event >= FSM_EVENT_MAX) return;

    fsm_transition_t transition = g_fsm_table[current_state][incoming_event];
    fsm_state_t next_state = transition.next_state;

    if (next_state != 0 || transition.action != NULL)
    {
        if (transition.action != NULL)
        {
            transition.action();
        }

        if (next_state != current_state)
        {
            printf("[FSM] State Transited: %d -> %d\n", current_state, next_state);
            current_state = next_state;
        }
    }
    else
    {
        printf("[FSM Warning] Invalid Event %d in State %d. Ignored.\n", incoming_event,
               current_state);
    }
}

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
    // 2. O(1) 狀態機超級迴圈 (修改這裡)
    while (1)
    {
        if (xQueueReceive(xFsmQueue, &incoming_event, pdMS_TO_TICKS(50)) == pdPASS)
        {
            // 【修改】直接呼叫剛剛抽離出來的函數
            fsm_process_event(incoming_event);
        }
        else
        {
            app_monitor_report_heartbeat(HEARTBEAT_BIT_FSM);
        }
    }
}

bool app_fsm_send_event_from_isr(fsm_event_t event)
{
    if (xFsmQueue == NULL)
    {
        return false;
    }
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    BaseType_t res = xQueueSendFromISR(xFsmQueue, &event, &xHigherPriorityTaskWoken);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    return (res == pdTRUE);
}
