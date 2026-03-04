#include <stdio.h>

#include "FreeRTOS.h"
#include "app/app_blink.h"
#include "app/app_button.h"
#include "app/app_display.h"
#include "app/app_sensor.h"
#include "app/app_storage.h"
#include "hal/hal_delay.h"
#include "hal/hal_init.h"
#include "task.h"

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    printf("FATAL: Stack overflow in task %s\n", pcTaskName);
    configASSERT(0);
}

int main(void)
{
    hal_init_system();

    // 透過 HAL 介面輪詢，完全不知道底層是哪家 MCU
    while (!hal_init_is_usb_connected())
    {
        hal_delay_ms(100);
    }
    hal_delay_ms(1000);

    printf("\n\n=== [SYSTEM START: FreeRTOS SMP Dual-Core] ===\n");

    // 【關鍵修復】Stack 統一放大到 1024 Words (4KB)，解決 CYW43 與 OLED 初始化的記憶體不足
    xTaskCreate(vAppDisplayTask, "OLED_Task", 1024, NULL, 3, NULL);
    xTaskCreate(vAppBlinkTask, "LED_Task", 1024, NULL, 2, NULL);
    xTaskCreate(vMonitorTask, "MON_Task", 1024, NULL, 1, NULL);
    xTaskCreate(vAppButtonTask, "BTN_Task", 1024, NULL, 4, NULL);
    // 注意：消費者優先權設得比生產者高，確保資料一生產出來立刻被處理
    xTaskCreate(vAppSensorProducerTask, "SNS_Prod", 1024, NULL, 2, NULL);
    xTaskCreate(vAppSensorConsumerTask, "SNS_Cons", 1024, NULL, 3, NULL);
    vTaskStartScheduler();

    while (1);
    return 0;
}