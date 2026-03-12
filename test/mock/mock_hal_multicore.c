// test/mock_hal_multicore.c
#include "hal_multicore.h"

// 假的啟動函式 (什麼都不做)
void hal_multicore_launch(core1_task_t task)
{
    (void)task;  // 避免 unused parameter警告
}

// 假的 Push (什麼都不做)
void hal_multicore_fifo_push(uint32_t data) { (void)data; }

// 假的 Pop (回傳 false 代表沒收到資料)
bool hal_multicore_fifo_pop(uint32_t *data)
{
    (void)data;
    return false;
}

// 假的檢查 (回傳 false)
bool hal_multicore_fifo_has_data(void) { return false; }