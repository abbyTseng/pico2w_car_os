#include "hal_fault.h"

#include <stdio.h>

#include "hardware/sync.h"
#include "pico/platform.h"  // 為了使用 __attribute__ 巨集

// 【新增】引入儲存模組，用來將紀錄寫入 Flash
#include "hal/hal_storage.h"

// 定義 Cortex-M 核心重啟所需的暫存器位址與密碼
#define SCB_AIRCR (*(volatile uint32_t *)0xE000ED0C)
#define SCB_AIRCR_VECTKEY (0x05FA << 16)
#define SCB_AIRCR_SYSRESETREQ (1 << 2)

// 1. 定義放在 No-Init RAM 的全域變數，重啟後資料才不會被清空
__attribute__((section(".uninitialized_data"))) crash_report_t g_crash_report;

// 2. C 語言處理函數 (由組合語言跳轉過來)
void hal_fault_c_handler(uint32_t *fault_stack)
{
    // 發生崩潰了，先把中斷全關，避免被其他 ISR 干擾
    __asm volatile("cpsid i");

    // 將 Stack 上的 8 個 Word 抄寫到 No-Init RAM
    g_crash_report.magic_word = 0xDEADBEEF;
    g_crash_report.fault_type = 1;  // 1 代表 HardFault
    g_crash_report.frame.r0 = fault_stack[0];
    g_crash_report.frame.r1 = fault_stack[1];
    g_crash_report.frame.r2 = fault_stack[2];
    g_crash_report.frame.r3 = fault_stack[3];
    g_crash_report.frame.r12 = fault_stack[4];
    g_crash_report.frame.lr = fault_stack[5];
    g_crash_report.frame.pc = fault_stack[6];  // 最關鍵的 Program Counter！
    g_crash_report.frame.xpsr = fault_stack[7];

    // 抄寫完畢，觸發軟體重啟 (Software Reset)，逃離死機狀態
    SCB_AIRCR = SCB_AIRCR_VECTKEY | SCB_AIRCR_SYSRESETREQ;

    // 確保編譯器不會優化掉後面的無窮迴圈
    __asm volatile("dsb");
    while (1);
}

// 3. 組合語言 Wrapper：攔截 HardFault 並判斷 Stack
__attribute__((naked)) void isr_hardfault(void)
{
    __asm volatile(
        " tst lr, #4                                \n" /* 測試 LR 的 Bit 2 (EXC_RETURN) */
        " ite eq                                    \n" /* If-Then-Else 條件執行 */
        " mrseq r0, msp                             \n" /* 如果是 0 (相等)，把 MSP 放進 R0 */
        " mrsne r0, psp                             \n" /* 如果是 1 (不相等)，把 PSP 放進 R0 */
        " ldr r1, handler_address_const             \n" /* 載入 C 處理函數的位址 */
        " bx r1                                     \n" /* 跳轉到 C 函數，此時 R0 就是 fault_stack
                                                           的指標 */
        " handler_address_const: .word hal_fault_c_handler \n");
}

// 4. 供 main.c 在開機時檢查的函數
void hal_fault_check_and_log_crash(void)
{
    if (g_crash_report.magic_word == 0xDEADBEEF)
    {
        printf("\n========================================\n");
        printf("🔥 [FATAL] PREVIOUS CRASH DETECTED! 🔥\n");
        printf("========================================\n");
        printf("Fault Type: %lu\n", g_crash_report.fault_type);
        printf("PC (Crash Address): 0x%08lX\n", g_crash_report.frame.pc);
        printf("LR (Return Address): 0x%08lX\n", g_crash_report.frame.lr);
        printf("========================================\n");

        // 【第二階段：安全落地】這時候系統已經安穩重啟，我們安全地將資料寫入 LittleFS
        common_status_t status = hal_storage_write_file(
            "crash.bin", (const uint8_t *)&g_crash_report, sizeof(crash_report_t));
        if (status == COMMON_OK)
        {
            printf("💾 [Storage] Crash report successfully saved to 'crash.bin'.\n\n");
        }
        else
        {
            printf("❌ [Storage] Failed to save crash report.\n\n");
        }

        // 清除 RAM 裡的標記，避免無限重啟循環
        g_crash_report.magic_word = 0x00000000;
    }
    else
    {
        // 如果 RAM 裡沒有 0xDEADBEEF，代表這是一次「冷開機 (Cold Boot, 拔掉 B+ 再插上)」
        // 或是正常的開機。我們去 Flash 裡面找看看有沒有歷史遺言！
        crash_report_t saved_report;
        common_status_t status =
            hal_storage_read_file("crash.bin", (uint8_t *)&saved_report, sizeof(crash_report_t));

        // 檢查檔案是否存在，且是否為我們寫入的結構格式
        if (status == COMMON_OK && saved_report.fault_type == 1)
        {
            printf("\n========================================\n");
            printf("📂 [HISTORY] SAVED CRASH LOG FOUND IN FLASH \n");
            printf("========================================\n");
            printf("PC (Crash Address): 0x%08lX\n", saved_report.frame.pc);
            printf("LR (Return Address): 0x%08lX\n", saved_report.frame.lr);
            printf("========================================\n\n");
        }
    }
}