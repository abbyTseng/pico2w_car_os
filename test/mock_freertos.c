#include "mock/event_groups.h"
#include "mock/queue.h"
#include "mock/semphr.h"
#include "mock/task.h"

// --- Dummy Task Functions ---
void vTaskDelay(TickType_t xTicksToDelay) { (void)xTicksToDelay; }
void vTaskDelayUntil(TickType_t *pxPreviousWakeTime, TickType_t xTimeIncrement)
{
    *pxPreviousWakeTime += xTimeIncrement;
}
TickType_t xTaskGetTickCount(void) { return 0; }
void vTaskSuspend(TaskHandle_t xTaskToSuspend) { (void)xTaskToSuspend; }
void vTaskSetTimeOutState(TimeOut_t *const pxTimeOut) { (void)pxTimeOut; }
BaseType_t xTaskCheckForTimeOut(TimeOut_t *const pxTimeOut, TickType_t *const pxTicksToWait)
{
    (void)pxTimeOut;
    (void)pxTicksToWait;
    return pdTRUE;  // 假裝已經 Timeout
}

// --- Dummy Queue Functions ---
QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize)
{
    (void)uxQueueLength;
    (void)uxItemSize;
    return (QueueHandle_t)1;  // 回傳一個非 NULL 的假指標
}
BaseType_t xQueueSend(QueueHandle_t xQueue, const void *pvItemToQueue, TickType_t xTicksToWait)
{
    (void)xQueue;
    (void)pvItemToQueue;
    (void)xTicksToWait;
    return pdPASS;
}
BaseType_t xQueueReceive(QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait)
{
    (void)xQueue;
    (void)pvBuffer;
    (void)xTicksToWait;
    return pdPASS;
}

// --- Dummy Event Group Functions ---
EventGroupHandle_t xEventGroupCreate(void) { return (EventGroupHandle_t)1; }
EventBits_t xEventGroupSetBits(EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet)
{
    (void)xEventGroup;
    return uxBitsToSet;
}
EventBits_t xEventGroupClearBits(EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToClear)
{
    (void)xEventGroup;
    return uxBitsToClear;
}
EventBits_t xEventGroupWaitBits(EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToWaitFor,
                                BaseType_t xClearOnExit, BaseType_t xWaitForAllBits,
                                TickType_t xTicksToWait)
{
    (void)xEventGroup;
    (void)xClearOnExit;
    (void)xWaitForAllBits;
    (void)xTicksToWait;
    return uxBitsToWaitFor;  // 直接假裝等到了
}

SemaphoreHandle_t xSemaphoreCreateMutex(void) { return (SemaphoreHandle_t)1; }
BaseType_t xSemaphoreTake(SemaphoreHandle_t xMutex, TickType_t xBlockTime)
{
    (void)xMutex;
    (void)xBlockTime;
    return pdTRUE;
}
BaseType_t xSemaphoreGive(SemaphoreHandle_t xMutex)
{
    (void)xMutex;
    return pdTRUE;
}