// test/test_ringbuffer.c
#include "common_ringbuffer.h"
#include "unity.h"

void setUp(void) {}
void tearDown(void) {}

// 測試 1: 基本的推入與彈出
void test_RingBuffer_Push_Pop_Basic(void)
{
    ring_buffer_t rb;
    uint8_t buffer_mem[10];
    uint8_t val;

    // 初始化
    common_ringbuffer_init(&rb, buffer_mem, 10);

    // 推入 0xAA
    TEST_ASSERT_TRUE(common_ringbuffer_push(&rb, 0xAA));

    // 彈出並檢查是否為 0xAA
    TEST_ASSERT_TRUE(common_ringbuffer_pop(&rb, &val));
    TEST_ASSERT_EQUAL_HEX8(0xAA, val);
}

// 測試 2: 滿了應該要拒絕寫入 (避免覆蓋)
void test_RingBuffer_Full_Reject(void)
{
    ring_buffer_t rb;
    uint8_t buffer_mem[3];  // 容量只有 3

    common_ringbuffer_init(&rb, buffer_mem, 3);

    // 填滿它
    common_ringbuffer_push(&rb, 1);
    common_ringbuffer_push(&rb, 2);
    // 注意：Ring Buffer 實作通常會保留一個空位來區分全滿/全空，所以容量 N 的 buffer 只能裝 N-1 個
    // 這裡我們假設我們的實作是 "浪費一個位元組" 的經典演算法

    // 第三個應該滿了 (如果 size=3, 實際可用 2)
    // 或者我們寫的實作是 size=3 就真的能裝 3 個？看我們怎麼寫。
    // 讓我們寫一個 "能裝滿 size-1" 的安全版本。

    TEST_ASSERT_FALSE(common_ringbuffer_push(&rb, 3));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_RingBuffer_Push_Pop_Basic);
    RUN_TEST(test_RingBuffer_Full_Reject);
    return UNITY_END();
}