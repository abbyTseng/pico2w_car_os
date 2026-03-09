/**
 * @file app_fsm.h
 * @brief Table-Driven Finite State Machine for Automotive Control
 * @note Strictly decoupled from hardware. Runs as an independent FreeRTOS task.
 */
#ifndef APP_FSM_H
#define APP_FSM_H

#include <stdbool.h>
#include <stdint.h>

#include "common/common_status.h"  // 假設你有定義基本的 common_status_t

/* -------------------------------------------------------------------------- */
/* 1. Enum Definitions (狀態與事件)                                             */
/* -------------------------------------------------------------------------- */

/** @brief 系統狀態列舉 */
typedef enum
{
    FSM_STATE_INIT = 0,
    FSM_STATE_IDLE,
    FSM_STATE_RUNNING,
    FSM_STATE_FAULT,
    FSM_STATE_OTA,
    FSM_STATE_MAX  // 用於定義 Array Size，必須放在最後
} fsm_state_t;

/** @brief 觸發狀態轉換的事件列舉 */
typedef enum
{
    FSM_EVENT_INIT_DONE = 0,
    FSM_EVENT_START,
    FSM_EVENT_STOP,
    FSM_EVENT_ERROR,
    FSM_EVENT_CLEAR_FAULT,  // 你的需求：從 Fault 回到 Idle
    FSM_EVENT_OTA_TRIGGER,
    FSM_EVENT_MAX  // 用於定義 Array Size，必須放在最後
} fsm_event_t;

/* -------------------------------------------------------------------------- */
/* 2. Public API (供 main.c 或其他 Task 呼叫)                                 */
/* -------------------------------------------------------------------------- */

/**
 * @brief 狀態機 Task 進入點
 * @param pvParameters FreeRTOS 傳遞的參數 (可傳入 Queue Handle 等)
 */
void vAppFsmTask(void *pvParameters);

/**
 * @brief 外部 Task 投遞事件給狀態機 (Thread-Safe)
 * @param event 要投遞的事件
 * @return true 投遞成功, false 投遞失敗 (Queue Full)
 */
bool app_fsm_send_event(fsm_event_t event);

/**
 * @brief 取得當前狀態 (供 Monitor Task 或顯示介面查詢)
 * @return fsm_state_t 當前狀態
 */
fsm_state_t app_fsm_get_current_state(void);

#endif /* APP_FSM_H */