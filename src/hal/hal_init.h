/**
 * @file hal_init.h
 * @brief HAL system initialization. Wraps SDK init (stdio, etc.).
 */

#ifndef HAL_INIT_H
#define HAL_INIT_H
#include <stdbool.h>
#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Initialize HAL / system (stdio, clocks, etc.).
     * Call once from main before app_run().
     */
    // 系統級初始化：負責 Clocks, Stdio, Watchdog 等
    void hal_init_system(void);
    // 新增：抽象化 USB 連線狀態檢查
    bool hal_init_is_usb_connected(void);
#ifdef __cplusplus
}
#endif

#endif /* HAL_INIT_H */
