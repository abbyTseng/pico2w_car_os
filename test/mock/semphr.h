// 檔案：test/mock/semphr.h
#ifndef INC_FREERTOS_SEMAPHORE_H
#define INC_FREERTOS_SEMAPHORE_H

#include "FreeRTOS.h"
#include "queue.h"

// 假型別
typedef void *SemaphoreHandle_t;

// 假動作：一律回傳成功 (pdTRUE) 或非 NULL 指標
#define xSemaphoreCreateMutex() ((SemaphoreHandle_t)1)
#define xSemaphoreTake(xMutex, xBlockTime) (pdTRUE)
#define xSemaphoreGive(xMutex) (pdTRUE)

#endif  // INC_FREERTOS_SEMAPHORE_H