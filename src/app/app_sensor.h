/**
 * @file app_sensor.h
 * @brief Application layer for simulated ADC sensor using FreeRTOS Queue.
 */
#ifndef APP_SENSOR_H
#define APP_SENSOR_H

#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 定義要透過 Queue 傳遞的資料結構 (Payload)
     * 這是我們封裝好的「包裹」，包含感測器 ID、數值與精確時間戳。
     */
    typedef struct
    {
        uint8_t sensor_id;
        uint16_t adc_value;
        uint32_t timestamp_ms;
    } SensorData_t;

    // 生產者：模擬 ADC 硬體定期採樣
    void vAppSensorProducerTask(void *pvParameters);

    // 消費者：接收並處理 ADC 數據
    void vAppSensorConsumerTask(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif /* APP_SENSOR_H */