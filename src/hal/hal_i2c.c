#include "hal_i2c.h"

#include "hardware/i2c.h"
#include "pico/stdlib.h"

// 引入 FreeRTOS API (僅限 .c 檔，不污染 HAL 介面)
#include "FreeRTOS.h"
#include "semphr.h"

#define I2C_INST i2c0
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5
#define BUS_RECOVERY_CLOCKS 9
#define RECOVERY_DELAY_US 5

// 【架構核心】I2C 專用互斥鎖
static SemaphoreHandle_t i2c_mutex = NULL;

static void hal_i2c_bus_recovery(void)
{
    // ... (保留你原本的 Bus Recovery 邏輯，無需更動) ...
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_SIO);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_SIO);
    gpio_set_dir(I2C_SCL_PIN, GPIO_OUT);
    gpio_set_dir(I2C_SDA_PIN, GPIO_IN);

    for (uint8_t i = 0; i < BUS_RECOVERY_CLOCKS; i++)
    {
        if (gpio_get(I2C_SDA_PIN)) break;
        gpio_put(I2C_SCL_PIN, 0);
        sleep_us(RECOVERY_DELAY_US);
        gpio_put(I2C_SCL_PIN, 1);
        sleep_us(RECOVERY_DELAY_US);
    }

    gpio_set_dir(I2C_SDA_PIN, GPIO_OUT);
    gpio_put(I2C_SDA_PIN, 0);
    sleep_us(RECOVERY_DELAY_US);
    gpio_put(I2C_SCL_PIN, 1);
    sleep_us(RECOVERY_DELAY_US);
    gpio_put(I2C_SDA_PIN, 1);
    sleep_us(RECOVERY_DELAY_US);

    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    i2c_init(I2C_INST, 100 * 1000);
}

void hal_i2c_init(uint32_t baudrate)
{
    // 【新增】初始化 Mutex (只建立一次)
    if (i2c_mutex == NULL)
    {
        i2c_mutex = xSemaphoreCreateMutex();
        // 嚴格防護：如果 RTOS Heap 滿了導致創建失敗，直接停機
        configASSERT(i2c_mutex != NULL);
    }

    i2c_init(I2C_INST, baudrate);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
}

hal_i2c_status_t hal_i2c_write_timeout(uint8_t addr, const uint8_t *src, size_t len,
                                       uint32_t timeout_us)
{
    if (src == NULL || len == 0) return HAL_I2C_ERROR;

    // 1. 嘗試獲取 Mutex (最多等待 50ms)
    // 這能保證 Core 0 和 Core 1 不會同時操作硬體
    if (i2c_mutex != NULL)
    {
        if (xSemaphoreTake(i2c_mutex, pdMS_TO_TICKS(50)) != pdTRUE)
        {
            return HAL_I2C_BUSY;  // 獲取鎖失敗，直接返回 Busy 讓上層決定是否重試
        }
    }

    // 2. 執行實體 I2C 寫入
    int result = i2c_write_timeout_us(I2C_INST, addr, src, len, false, timeout_us);

    hal_i2c_status_t final_status = HAL_I2C_OK;

    if (result == PICO_ERROR_TIMEOUT)
    {
        hal_i2c_bus_recovery();  // 恢復程序依然在 Mutex 的保護範圍內執行！
        final_status = HAL_I2C_TIMEOUT;
    }
    else if (result < 0)
    {
        final_status = HAL_I2C_ERROR;
    }

    // 3. 釋放 Mutex (統一出口，確保絕對不會發生 Deadlock)
    if (i2c_mutex != NULL)
    {
        xSemaphoreGive(i2c_mutex);
    }

    return final_status;
}