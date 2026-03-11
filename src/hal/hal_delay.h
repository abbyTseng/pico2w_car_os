/**
 * @file hal_delay.h
 * @brief Delay abstraction. App uses this instead of SDK sleep.
 */

#ifndef HAL_DELAY_H
#define HAL_DELAY_H

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdint.h>  // <--- 關鍵！少了這一行編譯器就不認識 uint32_t
    /**
     * Block for approximately ms milliseconds.
     * @param ms Delay in milliseconds
     */
    void hal_delay_ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* HAL_DELAY_H */
