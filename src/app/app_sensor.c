/**
 * @file app_sensor.c
 * @brief Implementation of Sensor Producer-Consumer using Queue.
 */

#include "app_sensor.h"

#include <stdio.h>
#include <stdlib.h>  // for rand() to generate mock data

#include "queue.h"

// 定義 Queue 的深度與單個元素大小
#define SENSOR_QUEUE_LENGTH 5
#define SENSOR_ITEM_SIZE sizeof(SensorData_t)

// 宣告 Queue Handle (這根管子的實體)
static QueueHandle_t xSensorQueue = NULL;

// --- 生產者 (Producer) ---
void vAppSensorProducerTask(void *pvParameters)
{
    (void)pvParameters;

    // 1. 建立 Queue (車規要求：所有 OS 物件應在系統啟動初期分配完畢)
    xSensorQueue = xQueueCreate(SENSOR_QUEUE_LENGTH, SENSOR_ITEM_SIZE);
    if (xSensorQueue == NULL)
    {
        printf("FATAL: Failed to create sensor queue!\n");
        vTaskDelete(NULL);  // 創建失敗則自毀 Task
    }

    SensorData_t mock_data = {0};  // 初始化結構體 (MISRA 規範)
    mock_data.sensor_id = 1;

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(1000);  // 1Hz 採樣頻率

    for (;;)
    {
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        // 模擬採樣資料
        mock_data.timestamp_ms = (uint32_t)pdTICKS_TO_MS(xTaskGetTickCount());
        mock_data.adc_value = (uint16_t)(rand() % 4096);  // 模擬 12-bit ADC (0~4095)

        // 2. 寄出包裹 (Send to Queue)
        // 參數：Queue Handle, 資料的指標, Timeout (0 代表滿了不等待，立刻回傳失敗)
        if (xQueueSend(xSensorQueue, &mock_data, 0) != pdPASS)
        {
            // 觸發滿載防禦 (Queue Overrun Handling)
            printf("[SENSOR_PRODUCER] ⚠️ WARNING: Queue Full! Data dropped.\n");
        }
        else
        {
            printf("[SENSOR_PRODUCER] ADC Sampled: %u. Sent to Queue.\n", mock_data.adc_value);
        }
    }
}

// --- 消費者 (Consumer) ---
void vAppSensorConsumerTask(void *pvParameters)
{
    (void)pvParameters;

    // 準備一個空的箱子來收包裹
    SensorData_t received_data = {0};

    for (;;)
    {
        // 確保 Queue 已經被 Producer 建立才能開始等
        if (xSensorQueue != NULL)
        {
            // 3. 接收包裹 (Receive from Queue)
            // 參數：Queue Handle, 接收緩衝區指標, portMAX_DELAY (死等，直到有包裹才醒來)
            if (xQueueReceive(xSensorQueue, &received_data, portMAX_DELAY) == pdPASS)
            {
                printf("[SENSOR_CONSUMER] ✅ Received! ID: %d, ADC: %u, Time: %lu ms\n",
                       received_data.sensor_id, received_data.adc_value,
                       (unsigned long)received_data.timestamp_ms);
            }
        }
        else
        {
            vTaskDelay(pdMS_TO_TICKS(100));  // Queue 還沒建好，稍微等一下
        }
    }
}