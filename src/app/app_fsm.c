#include "app_fsm.h"

#include <stdio.h>

#include "FreeRTOS.h"
#include "app_fsm_table.h"
#include "queue.h"

#define FSM_QUEUE_LENGTH 10
#define FSM_QUEUE_ITEM_SIZE sizeof(fsm_event_t)

static QueueHandle_t xFsmQueue = NULL;
static fsm_state_t current_state = FSM_STATE_INIT;

bool app_fsm_send_event(fsm_event_t event)
{
    if (xFsmQueue == NULL) return false;

    if (xFsmQueue == NULL) return false;

    // 嚴格規範：此 API 僅限 Task 層級呼叫。
    // 若未來有硬體中斷 (ISR) 需要發送 Event，我們會另外建立 app_fsm_send_event_from_isr()
    return (xQueueSend(xFsmQueue, &event, 0) == pdPASS);
}

fsm_state_t app_fsm_get_current_state(void)
{
    return current_state;  // 簡單回傳，若需極高嚴謹度可加上 Memory Barrier
}

void vAppFsmTask(void *pvParameters)
{
    (void)pvParameters;
    fsm_event_t incoming_event;

    // 1. 建立 Event Queue
    xFsmQueue = xQueueCreate(FSM_QUEUE_LENGTH, FSM_QUEUE_ITEM_SIZE);
    configASSERT(xFsmQueue);  // 車廠防呆：Queue 沒建成功直接 Crash

    printf("[FSM Task] Started. Current State: INIT\n");

    // 2. O(1) 狀態機超級迴圈 (完全沒有 switch-case!)
    while (1)
    {
        // 等待事件到來 (無限期阻塞，釋放 CPU 給其他 Task)
        if (xQueueReceive(xFsmQueue, &incoming_event, portMAX_DELAY) == pdPASS)
        {
            // 安全防護：檢查 Event 是否越界
            if (incoming_event >= FSM_EVENT_MAX) continue;

            // === 核心查表邏輯 O(1) ===
            fsm_transition_t transition = g_fsm_table[current_state][incoming_event];
            fsm_state_t next_state = transition.next_state;

            // 如果有定義合法的狀態轉移 (狀態有變，或者有明確的 action)
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
    }
}