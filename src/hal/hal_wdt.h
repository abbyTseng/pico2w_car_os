#ifndef HAL_WDT_H
#define HAL_WDT_H

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief 啟動硬體看門狗
 * @param timeout_ms 逾時時間 (建議 1500ms)
 */
void hal_wdt_init(uint32_t timeout_ms);

/**
 * @brief 餵狗 (刷新硬體看門狗計時器)
 * @note 整個系統中，只允許 app_monitor 呼叫此函數
 */
void hal_wdt_kick(void);

#endif  // HAL_WDT_H