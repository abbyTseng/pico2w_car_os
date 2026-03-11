/**
 * @file hal_delay.c
 * @brief Delay implementation. Wraps SDK sleep_ms.
 */

#include "hal_delay.h"

#include "pico/stdlib.h"

void hal_delay_ms(uint32_t ms) { sleep_ms(ms); }
