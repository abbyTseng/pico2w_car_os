#ifndef HAL_FAULT_H
#define HAL_FAULT_H

#include <stdint.h>

// 定義硬體自動壓入 Stack 的 Exception Frame 結構 (無 FPU 版本)
typedef struct
{
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;    // 發生 Exception 前的 Link Register (返回位址)
    uint32_t pc;    // Program Counter (發生崩潰的精確指令位址！)
    uint32_t xpsr;  // Program Status Register
} exception_frame_t;

// 定義我們要保存在 No-Init RAM 的崩潰報告結構
typedef struct
{
    uint32_t magic_word;      // 辨識碼，例如 0xDEADBEEF
    uint32_t fault_type;      // 記錄是 HardFault, MemManage 等
    exception_frame_t frame;  // 崩潰當下的暫存器快照
} crash_report_t;

// 供組合語言呼叫的 C 語言入口
void hal_fault_c_handler(uint32_t *fault_stack);

// 供 main.c 在開機時檢查是否有崩潰紀錄
void hal_fault_check_and_log_crash(void);

#endif /* HAL_FAULT_H */