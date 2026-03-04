/**
 * @file app_button.h
 * @brief Application layer for Button handling (Deferred Interrupt Processing).
 */
#ifndef APP_BUTTON_H
#define APP_BUTTON_H

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Deferred handler task for button press events.
     * Blocked indefinitely until notified by the ISR.
     * @param pvParameters FreeRTOS task parameters (unused).
     */
    void vAppButtonTask(void *pvParameters);

    /**
     * @brief Initialization for the Button App.
     * Registers the ISR callback with the HAL layer.
     */
    void app_button_init(void);

#ifdef __cplusplus
}
#endif

#endif /* APP_BUTTON_H */