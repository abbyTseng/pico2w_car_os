/* ========================================================================== */
/* 【核彈級強制宣告】確保 FreeRTOS 絕對認知這是一個雙核系統！                 */
/* ========================================================================== */
#ifndef configNUM_CORES
#define configNUM_CORES 2
#endif

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* -------------------------------------------------------------------------- */
/* 1. 核心基礎設定 (Core Settings)                                              */
/* -------------------------------------------------------------------------- */
#define configUSE_PREEMPTION 1
#define configUSE_TIME_SLICING 1
#define configCPU_CLOCK_HZ 150000000           // RP2350 預設時脈
#define configTICK_RATE_HZ ((TickType_t)1000)  // 1ms Tick
#define configMAX_PRIORITIES 5
#define configMINIMAL_STACK_SIZE 256  // 以 Word (4 bytes) 為單位
#define configMAX_TASK_NAME_LEN 16
#define configUSE_16_BIT_TICKS 0
#define configIDLE_SHOULD_YIELD 1

/* -------------------------------------------------------------------------- */
/* 2. SMP 雙核心專屬設定 (SMP Configuration for RP2350)                         */
/* -------------------------------------------------------------------------- */
/* configNUM_CORES 已經在最上面強制定義了，這裡不用再寫 */
#define configTICK_CORE 0                // 由 Core 0 負責處理 SysTick
#define configRUN_MULTIPLE_PRIORITIES 1  // 允許兩個核心跑不同優先級任務
#define configUSE_CORE_AFFINITY 0        // 允許我們把 Task 綁定在特定核心

/* -------------------------------------------------------------------------- */
/* 3. 記憶體管理 (Memory Management - Heap 4)                                   */
/* -------------------------------------------------------------------------- */
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define configSUPPORT_STATIC_ALLOCATION 0
#define configTOTAL_HEAP_SIZE (128 * 1024)  // 分配 128KB 給 RTOS

/* -------------------------------------------------------------------------- */
/* 4. 可靠度與除錯 (Reliability & Debugging hooks)                              */
/* -------------------------------------------------------------------------- */
#define configCHECK_FOR_STACK_OVERFLOW 2  // 嚴格的 Stack 溢出檢查 (對應 main.c 的 Hook)
#define configUSE_MALLOC_FAILED_HOOK 0
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0

/* Assertion 機制：這在車廠開發是標配，發生錯誤直接停機，不帶病運行 */
#define configASSERT(x)           \
    if ((x) == 0)                 \
    {                             \
        portDISABLE_INTERRUPTS(); \
        for (;;);                 \
    }

/* -------------------------------------------------------------------------- */
/* 4.5. 軟體計時器 (Software Timer) - 雙核 port.c 必須依賴它來傳遞中斷訊息 */
/* -------------------------------------------------------------------------- */
#define configUSE_TIMERS 1
#define configTIMER_TASK_PRIORITY (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH 10
#define configTIMER_TASK_STACK_DEPTH 256
/* 【終極修復】：開啟這兩個功能，讓 SMP 雙核能夠跨核心傳遞中斷訊號！ */
#define INCLUDE_xTimerPendFunctionCall 1
#define configUSE_DAEMON_TASK_STARTUP_HOOK 0
/* -------------------------------------------------------------------------- */
/* 5. API 包含設定 (API Inclusion)                                              */
/* -------------------------------------------------------------------------- */
#define INCLUDE_vTaskPrioritySet 1
#define INCLUDE_uxTaskPriorityGet 1
#define INCLUDE_vTaskDelete 1
#define INCLUDE_vTaskSuspend 1
#define INCLUDE_vTaskDelayUntil 1
#define INCLUDE_vTaskDelay 1
#define INCLUDE_xTaskGetSchedulerState 1
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#define INCLUDE_uxTaskGetStackHighWaterMark 1  // 日後看 Stack 用量必備

/* -------------------------------------------------------------------------- */
/* 6. 中斷向量映射 (Pico SDK 橋接)                                              */
/* -------------------------------------------------------------------------- */
/* 將 FreeRTOS 的底層 Porting 函數映射到 Pico SDK 的硬體中斷處理 */
#define vPortSVCHandler isr_svcall
#define xPortPendSVHandler isr_pendsv
#define xPortSysTickHandler isr_systick

#endif /* FREERTOS_CONFIG_H */