#ifndef APP_STORAGE_DEMO_H
#define APP_STORAGE_DEMO_H

#include "common/common_status.h"

common_status_t app_storage_log_boot(void);

// 新增 Task 介面
void vMonitorTask(void *pvParameters);

#endif