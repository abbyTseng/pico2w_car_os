#include "app/app_storage.h"
#include "unity.h"

// 宣告外部 Mock 控制函數
void mock_hal_storage_reset(void);
void mock_hal_storage_set_content(uint32_t val);
uint32_t mock_hal_storage_get_content(void);

void setUp(void)
{
    // 每個測試前都重置環境
    mock_hal_storage_reset();
}

void tearDown(void) {}

// 測試案例 1: 第一次開機 (模擬檔案不存在)
void test_AppStorage_FirstBoot_Should_StartAtOne(void)
{
    // 1. Arrange (準備): 什麼都不設定，預設 file_exists = false

    // 2. Act (執行)
    app_storage_log_boot();

    // 3. Assert (驗證)
    // 既然讀不到，count 預設 0，加 1 後寫入，應該是 1
    TEST_ASSERT_EQUAL_UINT32(1, mock_hal_storage_get_content());
}

// 測試案例 2: 正常開機 (模擬已經開機過 10 次)
void test_AppStorage_NormalBoot_Should_Increment(void)
{
    // 1. Arrange (準備): 設定 Flash 裡原本存的是 10
    mock_hal_storage_set_content(10);

    // 2. Act (執行)
    app_storage_log_boot();

    // 3. Assert (驗證)
    // 讀到 10，加 1，寫入應該是 11
    TEST_ASSERT_EQUAL_UINT32(11, mock_hal_storage_get_content());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_AppStorage_FirstBoot_Should_StartAtOne);
    RUN_TEST(test_AppStorage_NormalBoot_Should_Increment);
    return UNITY_END();
}