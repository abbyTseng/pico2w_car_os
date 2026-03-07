#ifndef HAL_I2C_H
#define HAL_I2C_H

#include <stdbool.h>
#include <stddef.h>  // for size_t
#include <stdint.h>

// I2C 狀態碼
typedef enum
{
    HAL_I2C_OK = 0,
    HAL_I2C_TIMEOUT,  // 發生逾時 (已觸發 Recovery)
    HAL_I2C_ERROR,    // 其他錯誤
    HAL_I2C_BUSY      // 匯流排忙碌
} hal_i2c_status_t;

// 初始化 I2C
void hal_i2c_init(uint32_t baudrate);

// 帶有 Timeout 機制的寫入 (核心功能)
hal_i2c_status_t hal_i2c_write_timeout(uint8_t addr, const uint8_t *src, size_t len,
                                       uint32_t timeout_us);

// 實驗專用：在持有鎖的狀態下強制佔用 CPU
void hal_i2c_lab_simulate_long_transfer(uint32_t work_ms);
#endif  // HAL_I2C_H