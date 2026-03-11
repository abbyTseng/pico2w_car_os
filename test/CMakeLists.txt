cmake_minimum_required(VERSION 3.14)

project(pico2_tests C)
enable_testing()
# ==============================================================================
# 1. 環境準備 (Unity 測試框架)
# ==============================================================================
include(FetchContent)
FetchContent_Declare(
  unity
  GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
  GIT_TAG        v2.5.2
)
FetchContent_MakeAvailable(unity)


# 設定標頭檔搜尋路徑 (讓測試程式找不到 .h 檔時去哪裡找)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/mock
    ${CMAKE_CURRENT_SOURCE_DIR}/../src        # <--- 重點！包含到 src 這一層
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/app
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/hal
    
)

# ==============================================================================
# 2. 既有的測試: test_blink (測試 App 層邏輯)
# ==============================================================================
# 如果你之前的 mock 檔還在，這個測試應該要能跑
# (備註：如果你 app_blink.c 加入了多核心功能，這裡可能缺 hal_multicore 的 mock，
#  如果報錯可以暫時註解掉下面這一段，或是補上 mock_hal_multicore.c)
add_executable(test_blink
    test_blink.c
    ../src/app/app_blink.c
    mock_hal_led.c 
    mock_hal_delay.c
    mock_hal_multicore.c
    mock_hal_gpio.c
    mock_freertos.c
    # 如果 app_blink.c 有用到 hal_gpio，這裡可能也要補上 mock/hardware/gpio.h 的路徑依賴
)
target_link_libraries(test_blink unity)
add_test(NAME test_blink COMMAND test_blink)

# ==============================================================================
# 4. Day 8 新增的測試: test_gpio_callback (一定要加這個 CI 才會跑)
# ==============================================================================
add_executable(test_gpio_callback
    test_gpio_callback.c
    #../src/hal/hal_gpio.c   # 這是我們要測的目標 (它會 include <pico/stdlib.h>)
)
target_link_libraries(test_gpio_callback unity)
add_test(NAME test_gpio_callback COMMAND test_gpio_callback)
# ==============================================================================
# 5. Day 7 的測試: test_ringbuffer
# ==============================================================================
# (如果你還沒寫 test_ringbuffer.c，這段可以先註解掉，不然會報錯找不到檔案)
# add_executable(test_ringbuffer
#    test_ringbuffer.c
#    ../src/common/common_ringbuffer.c
# )
# target_link_libraries(test_ringbuffer unity)
# add_test(NAME test_ringbuffer COMMAND test_ringbuffer)

# ==============================================================================
# 6. Day 9 新增的測試: test_hal_i2c (Bus Recovery)
# ==============================================================================
add_executable(test_hal_i2c
    test_hal_i2c.c
    mock_freertos.c
    # 注意：我們不加入 ../src/hal/hal_i2c.c，因為它已經被 include 在 .c 檔裡了 (白箱測試)
)
target_link_libraries(test_hal_i2c unity)
add_test(NAME test_hal_i2c COMMAND test_hal_i2c)

# ==============================================================================
# 6. Day 10 # 新增 App Storage 測試
# ==============================================================================

add_executable(test_app_storage
    test_app_storage.c
    mock_hal_storage.c          # 連結 Mock HAL
    mock_freertos.c
# 修改這裡：使用相對路徑，不要用 CMAKE_SOURCE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/app/app_storage.c
)
target_link_libraries(test_app_storage unity)
target_include_directories(test_app_storage PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
)
add_test(NAME test_app_storage COMMAND test_app_storage)


# ==============================================================================
# 7. Day 18 新增: test_app_monitor (軟體看門狗邏輯驗證)
# ==============================================================================
add_executable(test_app_monitor
    test_app_monitor.c
    ../src/app/app_monitor.c
    mock_freertos.c
    mock_hal_wdt.c          # <--- 【重點新增】掛載假看門狗
)
target_link_libraries(test_app_monitor unity)
target_include_directories(test_app_monitor PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
)
add_test(NAME test_app_monitor COMMAND test_app_monitor)