#include "app/app_monitor.h"
#include "unity.h"

void setUp(void) {}
void tearDown(void) {}

// 測試 1：所有 Task 都有回報，且時間符合預期 -> 允許餵狗
void test_app_monitor_all_tasks_alive_should_kick(void)
{
    uint32_t bits = HEARTBEAT_BIT_FSM | HEARTBEAT_BIT_DISPLAY;
    uint32_t elapsed_ms = 100;  // 假設預期每 100ms 檢查一次

    bool should_kick = app_monitor_evaluate_system_state(bits, elapsed_ms);
    TEST_ASSERT_TRUE_MESSAGE(should_kick, "All tasks reported, should kick WDT.");
}

// 測試 2：FSM 卡死沒回報 (Starvation) -> 拒絕餵狗
void test_app_monitor_fsm_dead_should_not_kick(void)
{
    uint32_t bits = HEARTBEAT_BIT_DISPLAY;  // 只有 Display 回報，FSM 丟失
    uint32_t elapsed_ms = 100;

    bool should_kick = app_monitor_evaluate_system_state(bits, elapsed_ms);
    TEST_ASSERT_FALSE_MESSAGE(should_kick, "FSM dead, must NOT kick WDT.");
}

// 測試 3：Window Watchdog 提早餵狗 (Early Kick) -> 拒絕餵狗
void test_app_monitor_early_kick_should_not_kick(void)
{
    uint32_t bits = HEARTBEAT_BIT_FSM | HEARTBEAT_BIT_DISPLAY;
    uint32_t elapsed_ms = 10;  // 才經過 10ms 就觸發檢查，表示 Task 陷入異常死迴圈

    bool should_kick = app_monitor_evaluate_system_state(bits, elapsed_ms);
    TEST_ASSERT_FALSE_MESSAGE(should_kick, "Early kick detected, must NOT kick WDT.");
}