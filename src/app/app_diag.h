/* src/app/app_diag.h */
#ifndef APP_DIAG_H
#define APP_DIAG_H

#include <stdbool.h>
#include <stdint.h>

#include "common/common_status.h"

// SAE J2012 範例: 0xC155 -> U0155 (通訊丟失)
#define DTC_I2C_BUS_ERROR 0xC155
#define DTC_STORAGE_FAILURE 0xC100

// UDS Status Bits (ISO 14229)
#define DTC_STATUS_BIT_TF (1 << 0)     // TestFailed
#define DTC_STATUS_BIT_TFTOC (1 << 1)  // TestFailedThisOperationCycle
#define DTC_STATUS_BIT_PDTC (1 << 2)   // PendingDTC
#define DTC_STATUS_BIT_CDTC (1 << 3)   // ConfirmedDTC

typedef struct
{
    uint32_t timestamp;
    uint16_t battery_mv;
    uint8_t fsm_state;
} diag_snapshot_t;

typedef struct
{
    uint16_t dtc_id;
    uint8_t status;
    uint16_t occurrence_count;
    diag_snapshot_t freeze_frame;
} dtc_record_t;

common_status_t app_diag_init(void);
// 報修介面：由 hal_i2c 或其他模組呼叫
void app_diag_report_event(uint16_t dtc_id, bool failed);
// 批次同步介面：由背景任務呼叫
void app_diag_sync_to_storage(void);

// 【新增】FreeRTOS Task 進入點
void vAppDiagTask(void *pvParameters);

#endif  // APP_DIAG_H