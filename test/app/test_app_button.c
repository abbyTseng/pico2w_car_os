#include "mock/FreeRTOS.h"
#include "unity.h"

// 宣告我們的魔法時間函數
extern void mock_freertos_set_ticks(TickType_t ticks);

// 【架構師絕招】直接 include 原始碼
#include "../../src/app/app_button.c"

void setUp(void)
{
    mock_freertos_set_ticks(0);  // 測試開始時，時間歸零
}

void tearDown(void) {}

// 測試：防彈跳邏輯是否生效
void test_app_button_debounce_logic(void)
{
    TickType_t last_wake_time = 0;
    uint32_t press_count = 0;

    // 第一次按下 (發生在 第 60 毫秒，60 - 0 > 50)
    mock_freertos_set_ticks(60);
    TickType_t current_time = xTaskGetTickCount();

    if ((current_time - last_wake_time) > pdMS_TO_TICKS(DEBOUNCE_TIME_MS))
    {
        press_count++;
        last_wake_time = current_time;  // 更新時間到 60
    }

    // 驗證第一次按壓應該被算入
    TEST_ASSERT_EQUAL(1, press_count);

    // 第二次按下：雜訊彈跳！(發生在 第 80 毫秒，80 - 60 = 20 <= 50)
    mock_freertos_set_ticks(80);
    current_time = xTaskGetTickCount();

    if ((current_time - last_wake_time) > pdMS_TO_TICKS(DEBOUNCE_TIME_MS))
    {
        press_count++;
        last_wake_time = current_time;
    }

    // 驗證防彈跳機制生效！(不應該增加，還是 1)
    TEST_ASSERT_EQUAL(1, press_count);

    // 第三次按下：真實按壓 (發生在 第 150 毫秒，150 - 60 = 90 > 50)
    mock_freertos_set_ticks(150);
    current_time = xTaskGetTickCount();

    if ((current_time - last_wake_time) > pdMS_TO_TICKS(DEBOUNCE_TIME_MS))
    {
        press_count++;
        last_wake_time = current_time;
    }

    // 驗證過了防彈跳時間後，可以再次觸發 (變成 2)
    TEST_ASSERT_EQUAL(2, press_count);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_app_button_debounce_logic);
    return UNITY_END();
}