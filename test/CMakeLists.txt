cmake_minimum_required(VERSION 3.13)
project(pico2w_car_os_tests)
enable_testing()

include(FetchContent)
FetchContent_Declare(
  unity
  GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity.git
  GIT_TAG        v2.6.0
)
FetchContent_MakeAvailable(unity)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/..)

# ==============================================================================
# 2. 【架構師魔法】將替身大軍打包成「靜態函式庫 (Static Library)」
# ==============================================================================
add_library(test_mocks STATIC
    mock/mock_freertos.c
    mock/mock_hal_delay.c
    mock/mock_hal_gpio.c
    mock/mock_hal_led.c
    mock/mock_hal_multicore.c
    mock/mock_hal_storage.c
    mock/mock_hal_wdt.c
    mock/mock_hal_i2c.c
)
target_include_directories(test_mocks PUBLIC 
    ${ROOT_DIR}/src 
    ${ROOT_DIR}/src/app 
    ${ROOT_DIR}/src/hal 
    ${ROOT_DIR}/src/common 
    ${CMAKE_CURRENT_SOURCE_DIR}/mock
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# ==============================================================================
# 3. 定義測試巨集
# ==============================================================================
macro(add_unity_test TEST_NAME SOURCE_FILE EXTRA_SOURCES)
    # 不再直接塞入 MOCK_SOURCES，只編譯測試檔本身
    add_executable(${TEST_NAME} ${SOURCE_FILE} ${EXTRA_SOURCES})
    
    # 讓連結器自己去 test_mocks 庫裡面挑需要的替身 (解決 multiple definition)
    target_link_libraries(${TEST_NAME} unity test_mocks)
    
    target_include_directories(${TEST_NAME} PRIVATE 
        ${ROOT_DIR}/src 
        ${ROOT_DIR}/src/app 
        ${ROOT_DIR}/src/hal 
        ${ROOT_DIR}/src/common 
        ${CMAKE_CURRENT_SOURCE_DIR}/mock
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endmacro()

# ==============================================================================
# 4. 掛載所有測試案例
# ==============================================================================

# --- Common Layer Tests ---
add_unity_test(test_common_ringbuffer common/test_common_ringbuffer.c "${ROOT_DIR}/src/common/common_ringbuffer.c")

# --- HAL Layer Tests ---
add_unity_test(test_hal_i2c hal/test_hal_i2c.c "")
add_unity_test(test_hal_gpio hal/test_hal_gpio.c "")

# --- App Layer Tests ---
add_unity_test(test_app_blink app/test_app_blink.c "${ROOT_DIR}/src/app/app_blink.c")
add_unity_test(test_app_storage app/test_app_storage.c "${ROOT_DIR}/src/app/app_storage.c")
add_unity_test(test_app_monitor app/test_app_monitor.c "${ROOT_DIR}/src/app/app_monitor.c")
add_unity_test(test_app_fsm app/test_app_fsm.c "")
add_unity_test(test_app_sync app/test_app_sync.c "")
add_unity_test(test_app_display app/test_app_display.c "")
add_unity_test(test_app_button app/test_app_button.c "")
add_unity_test(test_app_sensor app/test_app_sensor.c "")