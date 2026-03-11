#ifndef INC_FREERTOS_SEMAPHORE_H
#define INC_FREERTOS_SEMAPHORE_H
#include "FreeRTOS.h"
#include "queue.h"

typedef void *SemaphoreHandle_t;
// 改用函數宣告，停止巨集污染
SemaphoreHandle_t xSemaphoreCreateMutex(void);
BaseType_t xSemaphoreTake(SemaphoreHandle_t xMutex, TickType_t xBlockTime);
BaseType_t xSemaphoreGive(SemaphoreHandle_t xMutex);

#endif