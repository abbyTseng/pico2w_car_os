#include "unity.h"

// 1. 引入 Mock Headers (欺騙編譯器，讓它以為有 SDK)
#include "mock/hardware/gpio.h"
#include "mock/hardware/i2c.h"
#include "mock/pico/stdlib.h"

// 2. 白箱引入：直接把被測程式碼包進來
// 注意：這會讓編譯器編譯 hal_i2c.c，並使用上面定義的 Mock Header
#include "../src/hal/hal_i2c.c"

// --- Mock 狀態變數 ---
static int s_gpio_func_call_count = 0;
static int s_gpio_put_call_count = 0;
static int s_i2c_write_return_value = 0;  // 控制 Mock I2C 回傳什麼

// --- 實作 Mock 行為 (Spy) ---

void gpio_set_function(uint gpio, uint fn) { s_gpio_func_call_count++; }

void gpio_put(uint gpio, bool value)
{
    // 我們只在乎 SCL (Pin 5) 的 Toggle 動作
    if (gpio == 5)
    {
        s_gpio_put_call_count++;
    }
}

bool gpio_get(uint gpio)
{
    // [關鍵] 模擬 SDA (Pin 4) 永遠被拉低 (Stuck Bus)
    // 這樣 Recovery 機制就會跑滿 9 個 Clock
    if (gpio == 4) return 0;
    return 1;
}

// 模擬 I2C 寫入
int i2c_write_timeout_us(i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len, bool nostop,
                         uint timeout_us)
{
    return s_i2c_write_return_value;
}

// 其他必要的空實作 (避免 Link Error)
void gpio_set_dir(uint gpio, bool out) {}
void sleep_us(uint64_t us) {}
void sleep_ms(uint32_t ms) {}
// 回傳型別改為 uint，並隨便回傳一個值 (例如 baudrate) 來滿足編譯器
uint i2c_init(i2c_inst_t *i2c, uint baudrate) { return baudrate; }
void gpio_pull_up(uint gpio) {}
void gpio_init(uint gpio) {}

// --- 測試 Setup/Teardown ---
void setUp(void)
{
    s_gpio_func_call_count = 0;
    s_gpio_put_call_count = 0;
    s_i2c_write_return_value = 0;
}
void tearDown(void) {}

// --- 測試案例 ---

// 測試 1: 正常寫入，不應觸發 Recovery
void test_write_success_should_not_recover(void)
{
    s_i2c_write_return_value = 1;  // 模擬成功 (回傳 > 0)

    hal_i2c_status_t status = hal_i2c_write_timeout(0x3C, (uint8_t *)"", 1, 1000);

    TEST_ASSERT_EQUAL(HAL_I2C_OK, status);
    TEST_ASSERT_EQUAL(0, s_gpio_put_call_count);  // 沒發生 Toggle
}

// 測試 2: 寫入 Timeout，應觸發 9 Clocks Recovery
void test_write_timeout_should_trigger_recovery(void)
{
    s_i2c_write_return_value = PICO_ERROR_TIMEOUT;  // 模擬 Timeout

    hal_i2c_status_t status = hal_i2c_write_timeout(0x3C, (uint8_t *)"", 1, 1000);

    TEST_ASSERT_EQUAL(HAL_I2C_TIMEOUT, status);

    // 驗證是否切換了 GPIO 模式 (切成 SIO -> 切回 I2C，至少 2 次)
    TEST_ASSERT_TRUE(s_gpio_func_call_count >= 2);

    // 驗證 SCL 是否有 Toggle (Bit-Banging)
    // 9 個 Clocks * 2 (High/Low) + STOP bit...
    // 因為我們模擬 SDA 永遠 Stuck，所以它會跑滿迴圈
    TEST_ASSERT_TRUE(s_gpio_put_call_count >= 18);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_write_success_should_not_recover);
    RUN_TEST(test_write_timeout_should_trigger_recovery);
    return UNITY_END();
}