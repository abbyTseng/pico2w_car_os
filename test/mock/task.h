#ifndef MOCK_TASK_H
#define MOCK_TASK_H

#include "FreeRTOS.h"

// 原本的 Task 函數宣告
void vTaskDelay(TickType_t xTicksToDelay);
void vTaskDelayUntil(TickType_t *pxPreviousWakeTime, TickType_t xTimeIncrement);
TickType_t xTaskGetTickCount(void);
void vTaskSuspend(TaskHandle_t xTaskToSuspend);
void vTaskSetTimeOutState(TimeOut_t *const pxTimeOut);
BaseType_t xTaskCheckForTimeOut(TimeOut_t *const pxTimeOut, TickType_t *const pxTicksToWait);

// ============================================================================
// 【新增】補齊 app_button 與 app_sensor 會用到的進階 Task API 宣告
// ============================================================================
TaskHandle_t xTaskGetCurrentTaskHandle(void);
void vTaskNotifyGiveFromISR(TaskHandle_t xTaskToNotify, BaseType_t *pxHigherPriorityTaskWoken);
uint32_t ulTaskNotifyTake(BaseType_t xClearCountOnExit, TickType_t xTicksToWait);
void vTaskDelete(TaskHandle_t xTaskToDelete);

#endif  // MOCK_TASK_H