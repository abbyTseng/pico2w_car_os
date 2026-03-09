/**
 * @file app_fsm_table.h
 * @brief Internal types for the FSM table
 */
#ifndef APP_FSM_TABLE_H
#define APP_FSM_TABLE_H

#include "app_fsm.h"

/* 定義動作的函數指標 (Action Function Pointer) */
typedef void (*fsm_action_t)(void);

/* 定義狀態轉移結構 */
typedef struct
{
    fsm_state_t next_state;  // 發生此事件後要前往的狀態
    fsm_action_t action;     // 轉移過程中要執行的動作 (可為 NULL)
} fsm_transition_t;

/* 宣告對外提供的狀態矩陣 (定義在 app_fsm_table.c 中) */
extern const fsm_transition_t g_fsm_table[FSM_STATE_MAX][FSM_EVENT_MAX];

#endif /* APP_FSM_TABLE_H */