#include <stdbool.h>

#include "mock/FreeRTOS.h"
#include "mock/event_groups.h"
#include "unity.h"

// 宣告我們剛剛加進 mock_freertos.c 的重置函數與變數
extern void mock_freertos_reset_event_bits(void);
extern EventBits_t mock_event_bits;

// 【架構師絕招】直接 include .c 檔
#include "../../src/app/app_sync.c"

void setUp(void)
{
    xSyncEventGroup = NULL;
    mock_freertos_reset_event_bits();  // 每次測試前，清空假 EventGroup 的記憶
}

void tearDown(void) {}

void test_app_sync_init_creates_group(void)
{
    app_sync_init();
    TEST_ASSERT_NOT_NULL(xSyncEventGroup);
}

void test_app_sync_wait_for_all_success(void)
{
    app_sync_init();

    // 💡 關鍵修正：我們只設定低位元的 Ready Bits (0x000F)
    // 這樣就能完美避開高位元 (0x0700) 的 Error Bits 觸發區！
    app_sync_report_ready(0x000F);

    // 這次沒有錯誤干擾，Wait 一定會成功！
    bool result = app_sync_wait_for_all(1000);

    TEST_ASSERT_TRUE(result);
}

void test_app_sync_set_ready_executes_safely(void)
{
    app_sync_init();

    // 測試設定 0x01
    app_sync_report_ready(0x01);

    // 驗證 EventGroup 確實收到了 0x01
    TEST_ASSERT_EQUAL(0x01, mock_event_bits);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_app_sync_init_creates_group);
    RUN_TEST(test_app_sync_wait_for_all_success);
    RUN_TEST(test_app_sync_set_ready_executes_safely);
    return UNITY_END();
}