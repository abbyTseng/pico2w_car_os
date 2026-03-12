#include "mock/event_groups.h"
#include "mock/queue.h"
#include "mock/semphr.h"
#include "mock/task.h"

// --- 控制時間的魔法變數 ---
static TickType_t current_mock_tick = 0;
void mock_freertos_set_ticks(TickType_t ticks) { current_mock_tick = ticks; }
// 覆寫 xTaskGetTickCount
TickType_t xTaskGetTickCount(void) { return current_mock_tick; }

// --- 模擬 Task 通知 ---
static uint32_t simulated_notify_value = 0;
TaskHandle_t xTaskGetCurrentTaskHandle(void) { return (TaskHandle_t)1; }
void vTaskNotifyGiveFromISR(TaskHandle_t xTaskToNotify, BaseType_t *pxHigherPriorityTaskWoken)
{
    (void)xTaskToNotify;
    simulated_notify_value++;  // 收到通知就加一
    if (pxHigherPriorityTaskWoken) *pxHigherPriorityTaskWoken = pdTRUE;
}
uint32_t ulTaskNotifyTake(BaseType_t xClearCountOnExit, TickType_t xTicksToWait)
{
    (void)xTicksToWait;
    uint32_t val = simulated_notify_value;
    if (xClearCountOnExit) simulated_notify_value = 0;
    return val;
}
// --- Dummy Task Functions ---
void vTaskDelay(TickType_t xTicksToDelay) { (void)xTicksToDelay; }
void vTaskDelayUntil(TickType_t *pxPreviousWakeTime, TickType_t xTimeIncrement)
{
    *pxPreviousWakeTime += xTimeIncrement;
}
// 【新增】假裝刪除 Task
void vTaskDelete(TaskHandle_t xTaskToDelete) { (void)xTaskToDelete; }
void vTaskSuspend(TaskHandle_t xTaskToSuspend) { (void)xTaskToSuspend; }
void vTaskSetTimeOutState(TimeOut_t *const pxTimeOut) { (void)pxTimeOut; }
BaseType_t xTaskCheckForTimeOut(TimeOut_t *const pxTimeOut, TickType_t *const pxTicksToWait)
{
    (void)pxTimeOut;
    (void)pxTicksToWait;
    return pdFALSE;  // ❌ 之前預設回傳 pdTRUE 會導致計時迴圈直接放棄，改為 pdFALSE
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
EventBits_t mock_event_bits = 0;  // 【新增】記憶體變數

void mock_freertos_reset_event_bits(void) { mock_event_bits = 0; }

EventGroupHandle_t xEventGroupCreate(void) { return (EventGroupHandle_t)1; }

EventBits_t xEventGroupSetBits(EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet)
{
    (void)xEventGroup;
    mock_event_bits |= uxBitsToSet;  // 【升級】記錄被設定的 Bits
    return mock_event_bits;
}

EventBits_t xEventGroupClearBits(EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToClear)
{
    (void)xEventGroup;
    mock_event_bits &= ~uxBitsToClear;  // 【升級】清除指定的 Bits
    return mock_event_bits;
}

EventBits_t xEventGroupWaitBits(EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToWaitFor,
                                BaseType_t xClearOnExit, BaseType_t xWaitForAllBits,
                                TickType_t xTicksToWait)
{
    (void)xEventGroup;
    (void)xClearOnExit;
    (void)xWaitForAllBits;
    (void)xTicksToWait;

    // 就這麼簡單！大腦想看現在的狀態，我們就如實回傳目前的 Bits
    return mock_event_bits;
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

// 【新增假實作】永遠回傳成功
BaseType_t xQueueSendFromISR(QueueHandle_t xQueue, const void *pvItemToQueue,
                             BaseType_t *pxHigherPriorityTaskWoken)
{
    (void)xQueue;
    (void)pvItemToQueue;
    if (pxHigherPriorityTaskWoken)
    {
        *pxHigherPriorityTaskWoken = pdFALSE;
    }
    return pdTRUE;
}