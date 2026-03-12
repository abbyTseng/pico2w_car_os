#include <stdbool.h>
#include <stdint.h>

#include "mock/FreeRTOS.h"
#include "mock/queue.h"
#include "unity.h"

// ============================================================================
// 🛡️ 替身大軍 (Mocks & Stubs) 專區
// 我們在這裡攔截所有 app_fsm.c 向外呼叫的函數與變數，確保它能被 100% 隔離測試
// ============================================================================

// 1. 假裝我們有同步屏障 (永遠秒過)
bool app_sync_wait_for_all(uint32_t timeout_ms)
{
    (void)timeout_ms;
    return true;
}

// 2. 假裝我們有 Watchdog 監控回報 (什麼都不做)
void app_monitor_report_heartbeat(uint32_t task_bit) { (void)task_bit; }

// 3. 實作一個「專供測試用的假狀態表」，用來驗證 FSM 的 O(1) 引擎是否正常
#include "../src/app/app_fsm.h"
#include "../src/app/app_fsm_table.h"  // 取得 fsm_transition_t 的定義

const fsm_transition_t g_fsm_table[FSM_STATE_MAX][FSM_EVENT_MAX] = {
    [FSM_STATE_INIT] = {[FSM_EVENT_INIT_DONE] = {FSM_STATE_IDLE, NULL}},
    [FSM_STATE_IDLE] = {[FSM_EVENT_START] = {FSM_STATE_RUNNING, NULL}},
    [FSM_STATE_RUNNING] = {[FSM_EVENT_ERROR] = {FSM_STATE_FAULT, NULL}}};

// ============================================================================
// 【架構師絕招】替身準備好後，再把真實的受測程式碼 include 進來
#include "../../src/app/app_fsm.c"
// ============================================================================

void setUp(void)
{
    // 每次測試前，將狀態機重置為 INIT
    current_state = FSM_STATE_INIT;
}

void tearDown(void) {}

void test_fsm_should_transition_from_init_to_idle_on_start(void)
{
    // 根據我們的假表：INIT 收到 INIT_DONE 會進入 IDLE
    fsm_process_event(FSM_EVENT_INIT_DONE);
    TEST_ASSERT_EQUAL(FSM_STATE_IDLE, current_state);
}

void test_fsm_should_transition_from_idle_to_running_on_drive(void)
{
    // 根據我們的假表：IDLE 收到 START 會進入 RUNNING
    current_state = FSM_STATE_IDLE;
    fsm_process_event(FSM_EVENT_START);
    TEST_ASSERT_EQUAL(FSM_STATE_RUNNING, current_state);
}

void test_fsm_should_transition_to_error_state(void)
{
    // 根據我們的假表：RUNNING 收到 ERROR 會進入 FAULT
    current_state = FSM_STATE_RUNNING;
    fsm_process_event(FSM_EVENT_ERROR);
    TEST_ASSERT_EQUAL(FSM_STATE_FAULT, current_state);
}

void test_fsm_isr_send_should_return_true(void)
{
    // 騙它 queue 已經建好了，然後測試 ISR 投遞函數
    xFsmQueue = (QueueHandle_t)1;
    bool result = app_fsm_send_event_from_isr(FSM_EVENT_ERROR);
    TEST_ASSERT_TRUE(result);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_fsm_should_transition_from_init_to_idle_on_start);
    RUN_TEST(test_fsm_should_transition_from_idle_to_running_on_drive);
    RUN_TEST(test_fsm_should_transition_to_error_state);
    RUN_TEST(test_fsm_isr_send_should_return_true);
    return UNITY_END();
}