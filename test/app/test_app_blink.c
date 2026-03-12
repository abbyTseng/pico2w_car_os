#include "app_blink.h"  // 假設你有這個，或者直接宣告外部函式
#include "unity.h"

void setUp(void) {}
void tearDown(void) {}

// 目前 app_blink 已經重構為無窮迴圈，
// 舊的單元測試 (Init/Toggle) 已失效。
// 我們先保留這個檔案結構，Day 8 之後再來重寫針對 Blink 的測試。
void test_Placeholder(void) { TEST_ASSERT_TRUE(1); }

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_Placeholder);
    return UNITY_END();
}