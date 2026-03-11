/**
 * @file common_status.h
 * @brief Shared status codes for HAL and App layers.
 */

#ifndef COMMON_STATUS_H
#define COMMON_STATUS_H

typedef enum
{
    COMMON_OK = 0,
    COMMON_ERR = -1,
    COMMON_ERR_PARAM,
    COMMON_ERR_BUSY,
    COMMON_ERR_NOT_INIT
} common_status_t;

#endif /* COMMON_STATUS_H */
