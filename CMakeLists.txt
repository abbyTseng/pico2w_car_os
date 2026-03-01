cmake_minimum_required(VERSION 3.13)

# 1. 網路與路徑設定
set(FETCHCONTENT_BASE_DIR "${CMAKE_BINARY_DIR}/_deps")
set(PICO_BOARD pico2_w CACHE STRING "Board: pico2_w" FORCE)
set(PICOTOOL_FETCH_FROM_GIT_PATH ON CACHE BOOL "Use git path for picotool fetch" FORCE)
set(PICO_SDK_FETCH_FROM_GIT ON CACHE BOOL "Fetch Pico SDK from git if not found" FORCE)

# 2. 匯入 SDK (這行會自動把編譯器設定為 arm-none-eabi-gcc)
include(pico_sdk_import.cmake)

# 3. 宣告專案 (這時 CMake 才會正式去抓取 ARM 編譯器)
project(pico2w_car_os C CXX ASM)

# 4. 初始化 Pico SDK 工具與函數
pico_sdk_init()

set(CMAKE_C_STANDARD 11)
add_compile_options(-Wall -Wextra -Werror)

# 5. 設定 FreeRTOS 路徑
set(FREERTOS_KERNEL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external/FreeRTOS-Kernel)

# 6. 進入原始碼目錄
add_subdirectory(src)