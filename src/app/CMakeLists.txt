# App: Business logic. Hardware agnostic.

add_library(app STATIC
    app_blink.c
    app_display.c
    app_storage.c
    app_button.c
    app_sensor.c
)

target_include_directories(app PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# 加入 FreeRTOS-Kernel 與 freertos_config 的依賴
target_link_libraries(app PUBLIC
    hal
    common
    FreeRTOS-Kernel
    freertos_config
)