#include "app_fsm_table.h"

#include <stdio.h>

/* --- Action 函數實作 (Private) --- */
static void action_init_to_idle(void) { printf("[FSM Action] System Initialized. Ready.\n"); }

static void action_start_running(void) { printf("[FSM Action] Engine START. Running...\n"); }

static void action_stop_running(void) { printf("[FSM Action] Engine STOP. Idling...\n"); }

static void action_handle_fault(void) { printf("[FSM Action] FAULT DETECTED! Safing system...\n"); }

static void action_clear_fault(void)
{
    // 你的需求：從 Fault 回到 Idle 並 Print
    printf("[FSM Action] Fault Cleared. Returning to IDLE.\n");
}

/* --- 狀態轉移矩陣 (State Transition Matrix) --- */
// 使用 const 確保這個表存放在唯讀記憶體 (Flash/XIP) 中，提升安全性
const fsm_transition_t g_fsm_table[FSM_STATE_MAX][FSM_EVENT_MAX] = {
    // [當前狀態] = {
    //    [事件] = {目標狀態, 動作函數}
    // }

    [FSM_STATE_INIT] = {[FSM_EVENT_INIT_DONE] = {FSM_STATE_IDLE, action_init_to_idle},
                        // 【新增】開機同步屏障失敗 (Timeout/Error) 時的逃生路線
                        [FSM_EVENT_ERROR] = {FSM_STATE_FAULT, action_handle_fault}},

    [FSM_STATE_IDLE] = {[FSM_EVENT_START] = {FSM_STATE_RUNNING, action_start_running},
                        [FSM_EVENT_OTA_TRIGGER] = {FSM_STATE_OTA, NULL}},

    [FSM_STATE_RUNNING] = {[FSM_EVENT_STOP] = {FSM_STATE_IDLE, action_stop_running},
                           [FSM_EVENT_ERROR] = {FSM_STATE_FAULT, action_handle_fault}},

    [FSM_STATE_FAULT] =
        {
            [FSM_EVENT_CLEAR_FAULT] = {FSM_STATE_IDLE, action_clear_fault}
            // 在 FAULT 狀態下，無視 START / STOP 等其他事件 (查不到的預設為 0/NULL)
        },

    [FSM_STATE_OTA] = {
        // OTA 完成後通常直接 Reboot，這邊暫不處理
    }};