# ==========================================
# 先不要 include subdirectories! 
# 先定義 OS 層與 Config
# ==========================================

# 1. 建立 Config 虛擬目標
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# -----------------------------------------------------------------------------
# 【架構師終極解法】手動精準掛載 FreeRTOS 核心與 RP2040/RP2350 雙核移植層
# -----------------------------------------------------------------------------
add_library(FreeRTOS-Kernel STATIC
    ${FREERTOS_KERNEL_PATH}/tasks.c
    ${FREERTOS_KERNEL_PATH}/queue.c
    ${FREERTOS_KERNEL_PATH}/list.c
    ${FREERTOS_KERNEL_PATH}/timers.c
    ${FREERTOS_KERNEL_PATH}/event_groups.c
    ${FREERTOS_KERNEL_PATH}/stream_buffer.c
    ${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c
    # 手動掛載官方的移植層 C 檔
    ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/port.c
)

# 宣告精準的 Include 路徑，包含 portmacro.h 所在的路徑
target_include_directories(FreeRTOS-Kernel PUBLIC
    ${FREERTOS_KERNEL_PATH}/include
    ${FREERTOS_KERNEL_PATH}/portable/ThirdParty/GCC/RP2040/include
)

# 強制開啟 SMP 雙核模式
target_compile_definitions(FreeRTOS-Kernel PUBLIC configNUM_CORES=2)

# 【關鍵修復】：加入 hardware_exception，讓 port.c 能找到對應的標頭檔
target_link_libraries(FreeRTOS-Kernel PUBLIC 
    freertos_config 
    pico_stdlib 
    pico_multicore 
    hardware_exception 
)

# ==========================================
# OS 定義完成後，才引入我們的子模組 (HAL, App)
# 這樣它們才看得見 FreeRTOS-Kernel
# ==========================================
add_subdirectory(common)
add_subdirectory(hal)
add_subdirectory(app)

# ==========================================
# 主程式
# ==========================================
add_executable(${PROJECT_NAME}_main main.c)

pico_enable_stdio_usb(${PROJECT_NAME}_main 1)
pico_enable_stdio_uart(${PROJECT_NAME}_main 0)

target_compile_definitions(${PROJECT_NAME}_main PRIVATE configNUM_CORES=2)

target_link_libraries(${PROJECT_NAME}_main
    freertos_config
    FreeRTOS-Kernel         
    common
    hal
    app
)

pico_add_extra_outputs(${PROJECT_NAME}_main)