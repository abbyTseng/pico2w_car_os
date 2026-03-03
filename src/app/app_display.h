#ifndef APP_DISPLAY_H
#define APP_DISPLAY_H

#include <stdbool.h>

// SSD1306 初始化 (包含 I2C 測試)
bool app_display_init(void);

void app_display_test_once(void);
void vAppDisplayTask(void *pvParameters);
#endif  // APP_DISPLAY_H