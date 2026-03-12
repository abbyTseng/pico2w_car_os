#ifndef INC_FREERTOS_H
#define INC_FREERTOS_H

#include <stdbool.h>
#include <stdint.h>

// 騙過編譯器的假型別
typedef uint32_t TickType_t;
typedef uint32_t BaseType_t;
typedef uint32_t UBaseType_t;
typedef void *TaskHandle_t;
typedef void *QueueHandle_t;
typedef void *EventGroupHandle_t;
typedef uint32_t EventBits_t;

// 假常數
#define pdPASS 1
#define pdTRUE 1
#define pdFALSE 0
#define portMAX_DELAY 0xFFFFFFFF
#define pdMS_TO_TICKS(x) (x)
#define pdTICKS_TO_MS(x) (x)
#define portGET_CORE_ID() 0
#define configASSERT(x)
// 【新增巨集】騙過中斷 Context Switch 的呼叫
#define portYIELD_FROM_ISR(x) (void)(x)

typedef struct
{
    BaseType_t xOverflowCount;
    TickType_t xTimeOnEntering;
} TimeOut_t;

#endif  // INC_FREERTOS_H