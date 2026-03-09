/**
 * @file app_sync.h
 * @brief System Boot Synchronization Barrier (Decoupled RTOS Event Group)
 */
#ifndef APP_SYNC_H
#define APP_SYNC_H

#include <stdbool.h>
#include <stdint.h>

/* --- 1. 定義各模組的同步 Bit (使用 1 << X 防止數值重疊) --- */
/* Ready Bits (0~7) */
#define SYNC_BIT_DISPLAY_READY (1U << 0)
#define SYNC_BIT_STORAGE_READY (1U << 1)
#define SYNC_BIT_SENSOR_READY (1U << 2)
#define SYNC_ALL_READY_MASK \
    (SYNC_BIT_DISPLAY_READY | SYNC_BIT_STORAGE_READY | SYNC_BIT_SENSOR_READY)

/* Error Bits (8~15) */
#define SYNC_BIT_DISPLAY_ERROR (1U << 8)
#define SYNC_BIT_STORAGE_ERROR (1U << 9)
#define SYNC_BIT_SENSOR_ERROR (1U << 10)
#define SYNC_ANY_ERROR_MASK \
    (SYNC_BIT_DISPLAY_ERROR | SYNC_BIT_STORAGE_ERROR | SYNC_BIT_SENSOR_ERROR)

/* --- 2. Public API --- */

/**
 * @brief 建立同步屏障 (由 main.c 或 FSM 初始化時呼叫)
 */
void app_sync_init(void);

/**
 * @brief 各模組回報初始化完成
 * @param ready_bit 模組對應的 Ready Bit
 */
void app_sync_report_ready(uint32_t ready_bit);

/**
 * @brief 各模組回報初始化失敗 (觸發 Fast-Fail)
 * @param error_bit 模組對應的 Error Bit
 */
void app_sync_report_error(uint32_t error_bit);

/**
 * @brief 阻塞等待所有模組就緒 (供 FSM Task 呼叫)
 * @param timeout_ms 最大等待時間 (例如 3000ms)
 * @return true: 全部就緒 / false: 逾時或發生錯誤
 */
bool app_sync_wait_for_all(uint32_t timeout_ms);

#endif /* APP_SYNC_H */