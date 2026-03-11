# HAL: Hardware Abstraction Layer. Wraps RP2350 SDK only. No app logic.

add_library(hal STATIC
    hal_init.c
    hal_led.c
    hal_delay.c
    hal_dma.c
    hal_multicore.c
    hal_gpio.c
    hal_i2c.c
    hal_storage.c
    hal_fault.c
    hal_wdt.c
    # 直接加入 littlefs 源碼
    ${CMAKE_SOURCE_DIR}/external/littlefs/lfs.c
    ${CMAKE_SOURCE_DIR}/external/littlefs/lfs_util.c
)

# 【關鍵修正】針對 lfs.c 關閉 "未使用函數" 的警告
set_source_files_properties(
    ${CMAKE_SOURCE_DIR}/external/littlefs/lfs.c
    PROPERTIES COMPILE_FLAGS "-Wno-unused-function -Wno-error"
)

target_include_directories(hal PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src          # <--- 加入這行，它是 common 的父目錄
    ${CMAKE_SOURCE_DIR}/external/littlefs  # 讓編譯器找得到 lfs.h
)

target_link_libraries(hal PUBLIC
    pico_stdlib
    pico_cyw43_arch_none
    pico_multicore
    hardware_gpio
    hardware_dma
    hardware_i2c
    hardware_flash 
    hardware_sync
    common
    FreeRTOS-Kernel
    freertos_config
    pico_stdio_usb
)

