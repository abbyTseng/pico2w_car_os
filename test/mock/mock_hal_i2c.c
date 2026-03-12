#include "hal/hal_i2c.h"

// --- 暴露給測試檔控制的狀態變數 ---
int mock_i2c_write_call_count = 0;
// 修改為回傳你的專屬列舉型別
hal_i2c_status_t mock_i2c_write_returns[10];

// 提供一個 reset 函數，在每個測試開始前呼叫
void mock_hal_i2c_reset(void)
{
    mock_i2c_write_call_count = 0;
    for (int i = 0; i < 10; i++)
    {
        mock_i2c_write_returns[i] = HAL_I2C_OK;  // 預設回傳成功
    }
}

// --- 覆寫真實的 HAL API (完全對齊 hal_i2c.h) ---
hal_i2c_status_t hal_i2c_write_timeout(uint8_t addr, const uint8_t *src, size_t len,
                                       uint32_t timeout_us)
{
    // 假裝我們用掉了這些參數，避免編譯器警告
    (void)addr;
    (void)src;
    (void)len;
    (void)timeout_us;

    // 取出我們設定好的假回傳值
    hal_i2c_status_t ret = mock_i2c_write_returns[mock_i2c_write_call_count];

    // 推進呼叫次數
    if (mock_i2c_write_call_count < 9) mock_i2c_write_call_count++;

    return ret;
}

// 加上 uint32_t baudrate 參數
void hal_i2c_init(uint32_t baudrate) { (void)baudrate; }

void hal_i2c_bus_recovery(void) {}