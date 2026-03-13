/* src/app/app_monitor.h */
#ifndef APP_MONITOR_H
#define APP_MONITOR_H

#include <stdbool.h>
#include <stdint.h>

/* 定義受監控 Task 的心跳 Bits (使用 FreeRTOS Event Group) */
#define HEARTBEAT_BIT_FSM (1 << 0)
#define HEARTBEAT_BIT_DISPLAY (1 << 1)
#define HEARTBEAT_BIT_DIAG (1 << 2)  // 【新增】診斷任務專屬心跳

// 【更新】Mask 加入 DIAG
#define HEARTBEAT_MASK_ALL (HEARTBEAT_BIT_FSM | HEARTBEAT_BIT_DISPLAY | HEARTBEAT_BIT_DIAG)

/**
 * @brief 初始化監控模組 (建立 EventGroup)
 */
void app_monitor_init(void);

/**
 * @brief 供各個 Task 呼叫，回報自己還活著 (舉起心跳 Bit)
 * @param task_bit 對應的 HEARTBEAT_BIT_*
 */
void app_monitor_report_heartbeat(uint32_t task_bit);

/**
 * @brief 監控者 Task 實體
 * @param pvParameters FreeRTOS task 參數
 */
void vAppMonitorTask(void *pvParameters);

/**
 * @brief [Testable Logic] 核心判斷邏輯，抽離 RTOS 以利 Host 端單元測試
 * @param current_bits 目前收集到的心跳 bits
 * @param elapsed_ms 距離上次檢查經過的時間 (用於防範 Early Kick)
 * @return true 正常 (允許餵狗), false 異常 (拒絕餵狗)
 */
bool app_monitor_evaluate_system_state(uint32_t current_bits, uint32_t elapsed_ms);

#endif  // APP_MONITOR_H