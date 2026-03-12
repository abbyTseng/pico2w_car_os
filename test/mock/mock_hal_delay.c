#include <stdio.h>

#include "hal_delay.h"

// 假的延遲，什麼都不做，或者可以記錄呼叫次數
void hal_delay_ms(uint32_t ms) {
    // 在測試中我們不想真的等，所以這裡空著，或者 print 一下
    // printf("Mock Delay: %d ms\n", ms);
    (void)ms;
}