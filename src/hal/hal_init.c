/**
 * @file hal_init.c
 * @brief HAL init implementation. Only place that calls SDK stdio_init.
 */

#include "hal_init.h"

#include "pico/stdlib.h"
// 【關鍵修正】明確引入 USB 模組的標頭檔
#include "hal_storage.h"
#include "pico/stdio_usb.h"

void hal_init_system(void)
{
    stdio_init_all();
    // 【關鍵修復】初始化 Flash 與 LittleFS 檔案系統
    // 如果這裡沒叫，後面的 Task 一用檔案系統就會 Hard Fault
    hal_storage_init();
}
bool hal_init_is_usb_connected(void)
{
    // 封裝硬體 API，回傳 bool
    return stdio_usb_connected();
}