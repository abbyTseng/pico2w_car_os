#include <stdio.h>

#include "FreeRTOS.h"
#include "hal/hal_i2c.h"
#include "task.h"

// 真實模擬「佔用 CPU 運算」的時間 (排除被搶佔的時間)
static void simulate_heavy_work(uint32_t target_ms)
{
    uint32_t elapsed_active_ms = 0;
    TickType_t last_tick = xTaskGetTickCount();
    while (elapsed_active_ms < target_ms)
    {
        TickType_t now = xTaskGetTickCount();
        if (now != last_tick)
        {
            if ((now - last_tick) <= 2)
            {
                elapsed_active_ms += (now - last_tick);
            }
            last_tick = now;
        }
    }
}

void vLowPriorityTask(void *pv)
{
    (void)pv;
    vTaskDelay(pdMS_TO_TICKS(3000));  // 開機後等待穩定
    for (;;)
    {
        printf("[LP][T=%lu] 準備獲取 I2C，預計佔用 CPU 2000ms...\n", xTaskGetTickCount());
        hal_i2c_lab_simulate_long_transfer(2000);
        printf("[LP][T=%lu] 完成 I2C 操作並釋放鎖。\n", xTaskGetTickCount());
        vTaskDelay(pdMS_TO_TICKS(10000));  // 實驗結束，無限期休息
    }
}

void vMidPriorityTask(void *pv)
{
    int core_id = (int)pv;
    vTaskDelay(pdMS_TO_TICKS(4000));  // 在 LP 執行到一半時殺出
    for (;;)
    {
        printf("    [MP_%d][T=%lu] 啟動！雙核霸佔開始 (3000ms)...\n", core_id, xTaskGetTickCount());
        simulate_heavy_work(3000);
        printf("    [MP_%d][T=%lu] 運算結束，交出 CPU。\n", core_id, xTaskGetTickCount());
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

void vHighPriorityTask(void *pv)
{
    (void)pv;
    vTaskDelay(pdMS_TO_TICKS(5000));  // 在 MP 霸佔時請求資源
    uint8_t dummy = 0x00;
    for (;;)
    {
        printf("        [HP][T=%lu] 緊急請求 I2C...\n", xTaskGetTickCount());
        TickType_t t0 = xTaskGetTickCount();

        hal_i2c_status_t status = hal_i2c_write_timeout(0x3C, &dummy, 1, 2000);
        TickType_t t1 = xTaskGetTickCount();

        if (status == HAL_I2C_OK)
        {
            printf("        [HP][T=%lu] 獲取 I2C 成功！實際阻塞耗時: %lu ms\n", t1,
                   (t1 - t0) * portTICK_PERIOD_MS);
        }
        else
        {
            printf("        [HP][T=%lu] 獲取 I2C 失敗！阻塞耗時: %lu ms\n", t1,
                   (t1 - t0) * portTICK_PERIOD_MS);
        }

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}