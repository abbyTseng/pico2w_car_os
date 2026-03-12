#include "hal/hal_wdt.h"

// 假裝初始化，什麼都不做
void hal_wdt_init(uint32_t timeout_ms) { (void)timeout_ms; }

// 假裝餵狗，什麼都不做
void hal_wdt_kick(void) {}