#include <stdbool.h>
#include <string.h>  // 【修復 1】引入 memset 需要的標頭檔

#include "unity.h"

// --- Mocks ---
// 為了測試 app_diag，我們需要假造 hal_storage 的回傳值
#include "hal/hal_storage.h"
common_status_t mock_storage_status = COMMON_OK;
int mock_storage_write_count = 0;

common_status_t hal_storage_read_file(const char *filename, uint8_t *buffer, size_t size)
{
    // 模擬 Flash 是空的
    memset(buffer, 0, size);
    return COMMON_OK;
}

common_status_t hal_storage_write_file(const char *filename, const uint8_t *data, size_t size)
{
    mock_storage_write_count++;
    return mock_storage_status;
}

// 假造 Monitor 回報 (因為這個沒有在 test_mocks 裡)
void app_monitor_report_heartbeat(uint32_t task_bit) { (void)task_bit; }

// 【修復 2】把手動編寫的 xSemaphoreCreateMutex 等假函數刪除，
// 因為編譯器會自動去 libtest_mocks.a 裡面拿 mock_freertos.c 的實作！

// --- 引入受測模組 ---
#include "../../src/app/app_diag.c"

// --- 測試前準備 ---
void setUp(void)
{
    mock_storage_write_count = 0;
    mock_storage_status = COMMON_OK;
    memset(g_dtc_db, 0, sizeof(g_dtc_db));
    g_is_dirty = false;
    app_diag_init();
}

void tearDown(void) {}

// --- 測試矩陣 TM-01: 注入重複的 DTC ID ---
void test_diag_report_duplicate_event_increases_occurrence(void)
{
    // 第一次報告錯誤
    app_diag_report_event(0xC155, true);

    TEST_ASSERT_EQUAL_HEX16(0xC155, g_dtc_db[0].dtc_id);
    TEST_ASSERT_EQUAL_UINT16(1, g_dtc_db[0].occurrence_count);

    // 預期狀態：包含 Confirmed Bit (因為我們改為 1-Trip Fault)
    TEST_ASSERT_EQUAL_HEX8(DTC_STATUS_BIT_CDTC, g_dtc_db[0].status);
    TEST_ASSERT_TRUE(g_is_dirty);

    // 第二次報告相同錯誤
    app_diag_report_event(0xC155, true);

    // 驗證：次數變成 2，且沒有佔用陣列的下一個位子
    TEST_ASSERT_EQUAL_UINT16(2, g_dtc_db[0].occurrence_count);
    TEST_ASSERT_EQUAL_HEX16(0x0000, g_dtc_db[1].dtc_id);
}

// --- 測試: 狀態復原 (Healed) ---
void test_diag_report_event_healed(void)
{
    // 先發生錯誤
    app_diag_report_event(0xC155, true);
    // 強制加上 TestFailed Bit 模擬真實情況
    g_dtc_db[0].status |= DTC_STATUS_BIT_TF;

    // 後來測試通過
    app_diag_report_event(0xC155, false);

    // 驗證：TestFailed Bit 應該被清除，但 Confirmed Bit 保留 (歷史紀錄)
    TEST_ASSERT_EQUAL_HEX8(DTC_STATUS_BIT_CDTC, g_dtc_db[0].status);
}

// --- 測試: 背景寫入機制 ---
void test_diag_sync_to_storage_clears_dirty_flag(void)
{
    // 製造髒資料
    app_diag_report_event(0xC155, true);
    TEST_ASSERT_TRUE(g_is_dirty);

    // 呼叫同步函數
    app_diag_sync_to_storage();

    // 驗證：寫入成功後，dirty flag 應被清除，且呼叫了寫入函數
    TEST_ASSERT_FALSE(g_is_dirty);
    TEST_ASSERT_EQUAL(1, mock_storage_write_count);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_diag_report_duplicate_event_increases_occurrence);
    RUN_TEST(test_diag_report_event_healed);
    RUN_TEST(test_diag_sync_to_storage_clears_dirty_flag);
    return UNITY_END();
}