#include "hal_i2c.h"

#include "hardware/i2c.h"
#include "pico/stdlib.h"

// 引入 FreeRTOS API (僅限 .c 檔，不污染 HAL 介面)
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"  // 為了 xTaskGetTickCount

// ==========================================
// 🧪 LAB 控制開關：
// 0 = 使用 Binary Semaphore (引發優先權反轉災難)
// 1 = 使用 FreeRTOS Mutex (啟動優先權繼承救援)
#define LAB_USE_MUTEX 1
#define I2C_PRIORITY_LAB 1  // 開啟 Lab 模式
// ==========================================

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
#if LAB_USE_MUTEX
        i2c_mutex = xSemaphoreCreateMutex();
#else
        i2c_mutex = xSemaphoreCreateBinary();
        // Binary Semaphore 建立後預設是空的，必須先 Give 才能被 Take
        xSemaphoreGive(i2c_mutex);
#endif
        configASSERT(i2c_mutex != NULL);
    }

    i2c_init(I2C_INST, baudrate);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
}

#ifdef I2C_PRIORITY_LAB
void hal_i2c_lab_simulate_long_transfer(uint32_t work_ms)
{
    if (xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE)
    {
        uint32_t elapsed_active_ms = 0;
        TickType_t last_tick = xTaskGetTickCount();

        // 只計算真正持有 CPU 的時間！
        while (elapsed_active_ms < work_ms)
        {
            TickType_t now = xTaskGetTickCount();
            if (now != last_tick)
            {
                // 若差距大於 2，代表中間被高優先權任務搶佔了，不能計入工作進度
                if ((now - last_tick) <= 2)
                {
                    elapsed_active_ms += (now - last_tick);
                }
                last_tick = now;
            }
        }
        xSemaphoreGive(i2c_mutex);
    }
}
#endif

hal_i2c_status_t hal_i2c_write_timeout(uint8_t addr, const uint8_t *src, size_t len,
                                       uint32_t timeout_us)
{
    if (src == NULL || len == 0) return HAL_I2C_ERROR;

    // 1. 嘗試獲取 Mutex (最多等待 50ms)
    // 這能保證 Core 0 和 Core 1 不會同時操作硬體
    if (i2c_mutex != NULL)
    {
        TickType_t wait_time = portMAX_DELAY;  // 實驗模式：為了觀測反轉，無限制等待
        if (xSemaphoreTake(i2c_mutex, wait_time) != pdTRUE)
        {
            return HAL_I2C_BUSY;
        }
    }

    // 2. 執行實體 I2C 寫入
    // 2. 執行實體 I2C 寫入
    int result = i2c_write_timeout_us(I2C_INST, addr, src, len, false, timeout_us);

    hal_i2c_status_t final_status = HAL_I2C_OK;
    extern void app_diag_report_event(uint16_t dtc_id, bool failed);

    if (result == PICO_ERROR_TIMEOUT)
    {
        hal_i2c_bus_recovery();               // 恢復程序
        app_diag_report_event(0xC155, true);  // 0xC155 = DTC_I2C_BUS_ERROR
        final_status = HAL_I2C_TIMEOUT;
    }
    else if (result < 0)
    {
        app_diag_report_event(0xC155, true);
        final_status = HAL_I2C_ERROR;
    }
    else
    {
        // 成功！報告 Healed (取消當前錯誤狀態)
        app_diag_report_event(0xC155, false);
    }

    // 3. 釋放 Mutex (統一出口，確保絕對不會發生 Deadlock)
    if (i2c_mutex != NULL)
    {
        xSemaphoreGive(i2c_mutex);
    }

    return final_status;
}