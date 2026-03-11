#include "hal_wdt.h"

#include "hardware/watchdog.h"  // Pico SDK 底層 WDT

void hal_wdt_init(uint32_t timeout_ms)
{
    // 第二個參數 true 代表：如果在 Debugging 模式中斷，WDT 也會暫停，這對開發者非常友善！
    watchdog_enable(timeout_ms, true);
}

void hal_wdt_kick(void)
{
    watchdog_update();  // 刷新計時器
}