/**
 * @file common_types.h
 * @brief Shared type definitions. Hardware and SDK agnostic.
 */

#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include <stdbool.h>
#include <stdint.h>

/* Optional: shared integer types for portability */
typedef uint8_t common_uint8;
typedef uint16_t common_uint16;
typedef uint32_t common_uint32;

/**
 * @brief GPIO 事件回呼函式型別定義
 * @param pin 觸發的腳位編號
 * @param event 觸發事件 (例如 Falling Edge)
 */
typedef void (*hal_gpio_isr_callback_t)(uint32_t pin, uint32_t events);
#endif /* COMMON_TYPES_H */
