#include <stdio.h>

#include "hal_led.h"

// 這是假的 LED 裝置，用來記錄狀態
static LedState mock_led_state = LED_OFF;
static int init_call_count = 0;
static int toggle_call_count = 0;

// --- Mock 實作 ---
common_status_t mock_init(void) {
    init_call_count++;
    return COMMON_OK;
}

void mock_set_state(LedState state) { mock_led_state = state; }

void mock_toggle(void) {
    toggle_call_count++;
    mock_led_state = (mock_led_state == LED_ON) ? LED_OFF : LED_ON;
}

static const LedDevice mock_device = {
    .init = mock_init, .set_state = mock_set_state, .toggle = mock_toggle};

const LedDevice *hal_led_get_default(void) { return &mock_device; }

// --- 測試輔助函數 (讓測試程式可以檢查狀態) ---
int get_init_call_count(void) { return init_call_count; }
int get_toggle_call_count(void) { return toggle_call_count; }
LedState get_mock_led_state(void) { return mock_led_state; }
void reset_mock(void) {
    init_call_count = 0;
    toggle_call_count = 0;
    mock_led_state = LED_OFF;
}