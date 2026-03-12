#include <string.h>

#include "hal/hal_storage.h"
#include "pico/stdlib.h"  // <--- 引用妳寫的那張「假網」，它會自動提供 bool 和 uint32_t

// 模擬用的記憶體，假裝是 Flash 裡的檔案內容
static uint32_t virtual_flash_content = 0;
static bool file_exists = false;

// 測試控制用：重置模擬器狀態
void mock_hal_storage_reset(void)
{
    virtual_flash_content = 0;
    file_exists = false;
}

// 測試控制用：設定假檔案的內容
void mock_hal_storage_set_content(uint32_t val)
{
    virtual_flash_content = val;
    file_exists = true;
}

// 測試控制用：檢查最後寫入的值
uint32_t mock_hal_storage_get_content(void) { return virtual_flash_content; }

// --- 實作 HAL 介面 (Mock) ---

common_status_t hal_storage_init(void) { return COMMON_OK; }

common_status_t hal_storage_read_file(const char *path, uint8_t *buffer, size_t size)
{
    // 如果模擬設定為「檔案不存在」，就回傳錯誤
    if (!file_exists)
    {
        return COMMON_ERR;
    }

    // 模擬讀取：把變數內容複製到 buffer
    memcpy(buffer, &virtual_flash_content, size);
    return COMMON_OK;
}

common_status_t hal_storage_write_file(const char *path, const uint8_t *data, size_t size)
{
    // 模擬寫入：把 buffer 內容存到變數
    memcpy(&virtual_flash_content, data, size);
    file_exists = true;  // 寫入後檔案就存在了
    return COMMON_OK;
}