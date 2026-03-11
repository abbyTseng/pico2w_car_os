#include "FreeRTOS.h"
void vTaskDelay(TickType_t xTicksToDelay);
void vTaskDelayUntil(TickType_t *pxPreviousWakeTime, TickType_t xTimeIncrement);
TickType_t xTaskGetTickCount(void);
void vTaskSuspend(TaskHandle_t xTaskToSuspend);
void vTaskSetTimeOutState(TimeOut_t *const pxTimeOut);
BaseType_t xTaskCheckForTimeOut(TimeOut_t *const pxTimeOut, TickType_t *const pxTicksToWait);