#include <stdio.h>

#include "FreeRTOS.h"
#include "hal/hal_init.h"
#include "hal/hal_led.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "task.h"
// ... 保留你的 app_display, app_storage include ...

// --- 任務 A：LED 閃爍任務 ---
void vLedTask(void *pvParameters)
{
    (void)pvParameters;  // 告訴編譯器我們故意不使用這個參數
    const LedDevice *led = hal_led_get_default();

    while (1)
    {
        led->toggle();
        // 透過 get_core_num() 證明這個 Task 在哪顆核心跑
        printf("[Task LED] Toggling... Executing on Core: %d\n", get_core_num());

        // RTOS 的延遲，會交出 CPU 執行權，而不是死等！
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// --- 任務 B：系統監控任務 (故意用不同的 Delay 創造交錯) ---
void vMonitorTask(void *pvParameters)
{
    (void)pvParameters;  // 告訴編譯器我們故意不使用這個參數
    while (1)
    {
        printf("[Task Monitor] System OK. Executing on Core: %d\n", get_core_num());
        // vTaskDelay(pdMS_TO_TICKS(1500));
        // 你原本的 app_display_test_once() 可以放在這裡
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

// --- Stack 溢出防護 Callback (對應 configCHECK_FOR_STACK_OVERFLOW) ---
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;  // 告訴編譯器我們故意不使用這個參數
    printf("FATAL: Stack overflow in task %s\n", pcTaskName);
    // 嚴格標準：發生 Stack Overflow 直接觸發 Assert 停機，絕不帶病運行
    configASSERT(0);
}

int main(void)
{
    // 1. 初始化系統與硬體
    hal_init_system();
    while (!stdio_usb_connected())
    {
        sleep_ms(100);
    }
    sleep_ms(500);

    printf("\n\n=== [SYSTEM START] ===\n");
    const LedDevice *led = hal_led_get_default();
    led->init();

    printf("=== Starting FreeRTOS SMP ===\n");

    // 定義 Task Handles，我們需要它來設定屬性
    TaskHandle_t xLedTaskHandle = NULL;
    TaskHandle_t xMonitorTaskHandle = NULL;

    // 2. 建立 Tasks
    xTaskCreate(vLedTask, "LED_Task", 256, NULL, 1, &xLedTaskHandle);
    xTaskCreate(vMonitorTask, "MON_Task", 256, NULL, 1, &xMonitorTaskHandle);

    // 3. 【架構師核心動作：Core Affinity (綁核策略)】
    // FreeRTOS SMP 使用 Bitmask 來綁定核心：
    // (1 << 0) 即 0x01 代表 Core 0
    // (1 << 1) 即 0x02 代表 Core 1
    // if (xLedTaskHandle != NULL && xMonitorTaskHandle != NULL)
    //{
    //    vTaskCoreAffinitySet(xLedTaskHandle, (1 << 0));      // 綁定到 Core 0
    //    vTaskCoreAffinitySet(xMonitorTaskHandle, (1 << 1));  // 綁定到 Core 1
    //    printf(">> Core Affinity Set: LED->Core 0, Monitor->Core 1\n");
    //}

    // 4. 啟動排程器
    vTaskStartScheduler();

    // 如果跑到這裡，代表 Heap 記憶體不足，無法建立 Idle Task
    printf("Insufficient RAM for RTOS heap!\n");
    while (1);

    return 0;
}