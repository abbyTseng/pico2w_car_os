#include "app_display.h"

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "common/common_status.h"
#include "hal/hal_i2c.h"
#include "pico/stdlib.h"
#include "task.h"

#define OLED_ADDR 0x3C

/* * [初始化序列]
 * 這裡我們先不送 0xAF (Display ON)，
 * 等清除完記憶體後再手動開啟，避免開機瞬間看到雜訊。
 */
const uint8_t OLED_INIT_CMD[] = {
    0x00,        // Command Stream
    0xAE,        // Display OFF (先關閉)
    0xD5, 0x80,  // Clock Divide Ratio
    0xA8, 0x1F,  // Multiplex Ratio (128x32)
    0xD3, 0x00,  // Display Offset
    0x40,        // Start Line
    0x8D, 0x14,  // Charge Pump
    0x20, 0x02,  // Set Page Addressing Mode
    0xA1,        // Segment Re-map
    0xC8,        // COM Output Scan Direction
    0xDA, 0x02,  // COM Pins
    0x81, 0x7F,  // Contrast
    0xD9, 0xF1,  // Pre-charge
    0xDB, 0x40,  // VCOMH
    0xA4,        // Entire Display ON (Resume)
    0xA6         // Normal Display
    // 注意：這裡移除了 0xAF，我們稍後再開
};

/* * [修正] 強力清除函數
 * Timeout 提升至 50000 (50ms) 確保資料能傳完 128 Bytes
 */
static void app_display_clear(void)
{
    uint8_t zero_data[128 + 1];
    zero_data[0] = 0x40;               // Data Mode
    memset(&zero_data[1], 0x00, 128);  // 填滿 0

    for (int page = 0; page < 4; page++)
    {
        uint8_t set_pos[] = {
            0x00,
            (uint8_t)(0xB0 + page),  // Set Page Start Address
            0x00,                    // Set Lower Column Start Address (0)
            0x10                     // Set Higher Column Start Address (0)
        };
        hal_i2c_write_timeout(OLED_ADDR, set_pos, sizeof(set_pos), 50000);

        // [關鍵修正] Timeout 改為 50000 (50ms)
        // 100kHz 下傳輸 129 bytes 需要約 12ms，原本的 10ms 會導致右邊清不乾淨
        hal_i2c_write_timeout(OLED_ADDR, zero_data, sizeof(zero_data), 50000);
    }
}

static void app_display_show_day10(void)
{
    // 1. 定位：Page 1, Column 30
    uint8_t set_pos[] = {0x00, 0xB1, 0x0E, 0x11};
    hal_i2c_write_timeout(OLED_ADDR, set_pos, sizeof(set_pos), 50000);

    // 2. 標準 5x7 字型數據
    uint8_t pattern[] = {0x40,  // Data Start
                                // D
                         0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00,
                         // a
                         0x20, 0x54, 0x54, 0x78, 0x00,
                         // y
                         0x9C, 0xA0, 0xA0, 0x7C, 0x00,
                         // Space
                         0x00, 0x00, 0x00,
                         // 1
                         0x00, 0x42, 0x7F, 0x40, 0x00,
                         // 0
                         0x3E, 0x51, 0x49, 0x3E, 0x00};

    hal_i2c_write_timeout(OLED_ADDR, pattern, sizeof(pattern), 50000);
}

static void app_display_on(void)
{
    uint8_t cmd[] = {0x00, 0xAF};  // Display ON
    hal_i2c_write_timeout(OLED_ADDR, cmd, sizeof(cmd), 10000);
}

bool app_display_init(void)
{
    hal_i2c_init(100 * 1000);

    hal_i2c_status_t i2c_status =
        hal_i2c_write_timeout(OLED_ADDR, OLED_INIT_CMD, sizeof(OLED_INIT_CMD), 50000);

    if (i2c_status != HAL_I2C_OK)
    {
        printf("[App] OLED Init Failed: %d\n", i2c_status);
        return false;
    }

    // 1. 此時螢幕還是黑的 (因為還沒送 0xAF)
    // 2. 先把記憶體裡的雜訊清乾淨
    app_display_clear();

    // 3. 寫入 "Day 10"
    app_display_show_day10();

    // 4. 最後才打開螢幕，呈現完美的畫面
    app_display_on();

    printf("[App] OLED Init Success.\n");
    return true;
}

void app_display_test_once(void) { app_display_show_day10(); }

void vAppDisplayTask(void *pvParameters)
{
    (void)pvParameters;

    printf("[App Display] Initializing OLED...\n");
    // 這裡的 init 底層會呼叫 hal_i2c_init，進而建立 Mutex
    if (!app_display_init())
    {
        printf("[App Display] OLED Init Failed. Suspending Task.\n");
        vTaskSuspend(NULL);  // 初始化失敗就掛起自己，不要空轉浪費 CPU
    }

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(100);  // 100ms 週期 (10Hz 高頻刷新)

    while (1)
    {
        // 這裡會呼叫 hal_i2c_write，底層會自動 Take/Give Mutex！
        app_display_show_day10();

        // 註解掉 printf 避免洗畫面，但實作上它確實在跑
        // printf("[Task Display] Refreshed on Core: %d\n", portGET_CORE_ID());

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}