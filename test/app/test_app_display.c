#include <stdbool.h>

#include "hal/hal_i2c.h"
#include "mock/FreeRTOS.h"
#include "unity.h"

// 引入假手的控制開關
extern int mock_i2c_write_call_count;
extern hal_i2c_status_t mock_i2c_write_returns[10];
extern void mock_hal_i2c_reset(void);

// 給 app_display_init 呼叫的假看門狗回報
void app_monitor_report_heartbeat(uint32_t task_bit) { (void)task_bit; }

// 【架構師絕招】退兩層，引入真實的大腦 (受測代碼)
#include "../../src/app/app_display.c"

void setUp(void)
{
    // 每次測試前，把假手的狀態歸零
    mock_hal_i2c_reset();
}

void tearDown(void) {}

// 測試 1：一路順風的情況
void test_app_display_init_success(void)
{
    // 設定假手：第一次呼叫 (傳送 OLED_INIT_CMD) 回傳成功
    mock_i2c_write_returns[0] = HAL_I2C_OK;
    // 後續呼叫 (清空螢幕等) 也都預設回傳成功...

    // 呼叫真實的大腦！
    bool result = app_display_init();

    // 驗證大腦的決策：應該要回傳 true
    TEST_ASSERT_TRUE(result);
}

// 測試 2：出師不利，I2C 斷線的情況
void test_app_display_init_fails_on_first_timeout(void)
{
    // 設定假手：第一次呼叫 (傳送 OLED_INIT_CMD) 就遇到硬體超時
    mock_i2c_write_returns[0] = HAL_I2C_TIMEOUT;

    // 呼叫真實的大腦！
    bool result = app_display_init();

    // 驗證大腦的決策：
    // 1. 大腦應該要察覺到錯誤，並回傳 false
    TEST_ASSERT_FALSE(result);
    // 2. 因為第一次就失敗退出了，所以假手只會被呼叫 1 次！不會去執行後面的清空螢幕邏輯
    TEST_ASSERT_EQUAL(1, mock_i2c_write_call_count);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_app_display_init_success);
    RUN_TEST(test_app_display_init_fails_on_first_timeout);
    return UNITY_END();
}