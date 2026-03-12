#include "unity.h"

// 1. 先引入 Mock Headers (定義 uint 和 SDK 原型)
#include "mock/hardware/gpio.h"
#include "mock/pico/stdlib.h"

// 2. 白箱測試：引入原始碼
// (這時候 hal_gpio.c 看到的 uint 已經被上面定義好了，所以不會報錯)
#include "../src/hal/hal_gpio.c"

// =========================================================
// 🚧 Mock SDK 實作 (Stubs)
// 為了讓 hal_gpio.c 能通過連結，我們必須實作它呼叫的 SDK 函式
// =========================================================

void gpio_init(uint gpio) { (void)gpio; }
void gpio_set_dir(uint gpio, bool out)
{
    (void)gpio;
    (void)out;
}
void gpio_pull_up(uint gpio) { (void)gpio; }

// 這是最重要的一個，因為 hal_gpio_init_input 會呼叫它
void gpio_set_irq_enabled_with_callback(uint gpio, uint32_t events, bool enabled,
                                        gpio_irq_callback_t callback)
{
    (void)gpio;
    (void)events;
    (void)enabled;
    (void)callback;
    // 在單元測試中，我們不需要真的註冊硬體中斷
    // 我們會直接呼叫 _internal_gpio_isr 來模擬
}

// =========================================================
// 🧪 測試程式碼
// =========================================================

// 模擬 App 層的 Callback 函式 (Mock)
volatile int callback_triggered_count = 0;

void mock_app_handler(uint32_t gpio, uint32_t events)
{
    (void)gpio;
    (void)events;
    callback_triggered_count++;
}

// Unity Setup (每個測試前重置狀態)
void setUp(void)
{
    // 注意：_app_callback 是 hal_gpio.c 裡的 static 變數
    // 因為我們用了白箱測試 (#include .c)，所以可以直接存取它！
    _app_callback = NULL;
    callback_triggered_count = 0;
}

void tearDown(void) {}

// --- 測試案例 ---

// 測試 1: 檢查 set_callback 是否正確儲存函式指標
void test_hal_gpio_set_callback_should_store_function_pointer(void)
{
    // Act
    hal_gpio_set_isr_callback(mock_app_handler);

    // Assert: 檢查 static 變數是否等於我們傳進去的函式
    TEST_ASSERT_EQUAL_PTR(mock_app_handler, _app_callback);
}

// 測試 2: 模擬硬體中斷觸發 ISR，檢查是否轉發給 App
void test_internal_isr_should_trigger_app_callback(void)
{
    // Arrange
    hal_gpio_set_isr_callback(mock_app_handler);

    // Act: 直接呼叫 static ISR (模擬硬體行為)
    // 因為 mock/pico/stdlib.h 定義了 uint，這裡編譯器就能正確識別參數了
    _internal_gpio_isr(22, GPIO_IRQ_EDGE_FALL);

    // Assert
    TEST_ASSERT_EQUAL_INT(1, callback_triggered_count);
}

// 測試 3: 如果沒註冊 Callback，ISR 不應該當機或亂叫
void test_internal_isr_should_do_nothing_if_no_callback(void)
{
    // Arrange
    _app_callback = NULL;

    // Act
    _internal_gpio_isr(22, GPIO_IRQ_EDGE_FALL);

    // Assert
    TEST_ASSERT_EQUAL_INT(0, callback_triggered_count);
}

// 程式入口
int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_hal_gpio_set_callback_should_store_function_pointer);
    RUN_TEST(test_internal_isr_should_trigger_app_callback);
    RUN_TEST(test_internal_isr_should_do_nothing_if_no_callback);
    return UNITY_END();
}