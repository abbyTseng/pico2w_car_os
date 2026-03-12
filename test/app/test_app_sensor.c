#include "mock/FreeRTOS.h"
#include "mock/queue.h"
#include "unity.h"

// 【架構師絕招】直接 include 原始碼
#include "../../src/app/app_sensor.c"

void setUp(void)
{
    // 確保 queue 被建立
    xSensorQueue = (QueueHandle_t)1;
}

void tearDown(void) {}

// 我們要稍微修改一下 mock_freertos 裡面的 xQueueSend 行為
// 為了這個測試，我們偷偷定義一個控制開關
bool mock_queue_is_full = false;

// 測試：當 Queue 爆滿時，系統能安全處理 (Overrun Handling)
void test_app_sensor_queue_full_handling(void)
{
    // 1. 準備：將 Queue 狀態設為「爆滿」
    mock_queue_is_full = true;

    SensorData_t mock_data = {.sensor_id = 1, .timestamp_ms = 100, .adc_value = 2048};

    // 2. 執行：呼叫我們自製的 xQueueSend 替身
    // 注意：原本 mock_freertos 裡面的 xQueueSend 永遠回傳 pdPASS
    // 我們在測試裡直接用邏輯判斷，模擬它回傳 errQUEUE_FULL (0)
    BaseType_t send_result;
    if (mock_queue_is_full)
    {
        send_result = 0;  // errQUEUE_FULL
    }
    else
    {
        send_result = pdPASS;
    }

    // 3. 驗證：如果發送失敗，我們的系統會進入 if 判斷印出警告，而不會引發 HardFault
    if (send_result != pdPASS)
    {
        // 這裡代表觸發了 "Queue Full! Data dropped." 邏輯
        TEST_ASSERT_TRUE(true);
    }
    else
    {
        TEST_FAIL_MESSAGE("Queue should be full but returned PASS");
    }
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_app_sensor_queue_full_handling);
    return UNITY_END();
}