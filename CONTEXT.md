# Context: Pico 2 Automotive Firmware Project

## 1. Project Overview
* **Project Name:** pico2_automotive_firmware
* **Target Hardware:** Raspberry Pi Pico 2 W (RP2350 + CYW43) + SSD1306 OLED
* **Language:** C (Standard C99/C11)
* **Build System:** CMake + Docker (Standardized Build Env)
* **Testing:** Unity Framework (Unit Test) + Saleae/Oscilloscope (Physical Test)
* **Current Phase:** Phase 3 - RTOS Task Design & RMS (Starting Day 12)

## 2. Architecture & Design Patterns (Layered Architecture)
We have refactored the system into a strict 3-layer architecture to ensure decoupling and testability.

### A. Main Layer (`src/main.c`)
* **Role:** System Scheduler & Configurator.
* **Responsibilities:**
    * Initializes System HAL (`hal_init_system`).
    * Initializes Drivers (`hal_led`, `hal_i2c`, `hal_storage`).
    * Injects dependencies into App Layer.
    * Starts FreeRTOS Scheduler (`vTaskStartScheduler`).
* **Constraint:** **NO** direct hardware manipulation logic allowed here. Super loop replaced by RTOS Tasks.

### B. App Layer (`src/app/`)
* **Role:** Business Logic (e.g., `app_display`, `app_storage`).
* **Responsibilities:**
    * Implements feature logic (e.g., OLED UI state machine, Boot counting).
    * Calls HAL interfaces using `common_status_t`.
* **Constraint:** Hardware-agnostic. Should run on any MCU if HAL is provided.

### C. HAL Layer (`src/hal/`)
* **Role:** Hardware Abstraction.
* **Key Modules:**
    * **`hal_i2c`:** Implements "9-Clock Recovery" and physical timing timeouts.
    * **`hal_led`:** Abstracts Pico 2W's CYW43 wireless LED control behind a generic interface.
    * **`hal_gpio`:** Handles interrupts and callback registration.
    * **`hal_storage`:** Abstraction for LittleFS on Flash with XIP protection.

---

## 3. CI/CD Architecture (Dual-Track & Shift-Left Strategy)

*Note: Currently in OS Bring-up transition. CI/CD pipelines are expected to fail until FreeRTOS mocking is implemented.*

### A. Local Guardrail (The "Hook")
* **Trigger:** `git commit` via `.git/hooks/pre-commit` (Implementation delayed to Phase 4).
* **Strategy:** "Shift-Left Testing" to catch compilation and unit test errors before pushing.
* **Current Status:** Temporarily disabled to allow committing OS integration code without triggering false-positive test failures due to missing FreeRTOS definitions in the x86 test environment.

### B. Cloud Factory (GitHub Actions)
* **Trigger:** `git push` (`.github/workflows/main.yml`)
* **Actions:** Lint (Cppcheck), Test (CTest), Build (`.uf2`).
* **Current Status:** Red (Failed) ❌. This is an accepted technical debt while we migrate the test suite to recognize FreeRTOS headers and mock the RTOS scheduler.

---

## 4. Testing Strategy (Unity Framework)

### A. Mocking Strategy
* **Level 1: Testing App Logic**
    * **Goal:** Verify App logic without hardware.
    * **Method:** Link against `mock_hal_xxx.c`.
    * **CMake:** Add mock source files to `add_executable`.

* **Level 2: Testing HAL Logic (The "Mock SDK" Approach)**
    * **Goal:** Verify HAL interacts correctly with Pico SDK functions.
    * **Method:** Use `test/mock/` headers to simulate SDK behavior in Docker (x86).

### B. White-box Testing (Crucial for Internal Logic)
To test `static` functions (ISRs, internal state machines) or `static` variables:
1.  **Technique:** Directly `#include "../src/hal/hal_xxx.c"` inside the test file.
2.  **Rule:** **DO NOT** add the source file to `add_executable` in CMake to avoid multiple definitions.

---

## 5. Key Directory Structure
.
├── .github
│   └── workflows
│       └── main.yml       # Cloud CI/CD Pipeline
├── CONTEXT.md
├── Dockerfile
├── docker-compose.yml
├── external
│   ├── FreeRTOS-Kernel    # [Added Day 11] v11.1.0 SMP Kernel
│   └── littlefs           
├── src
│   ├── CMakeLists.txt     # [Modified Day 11] Custom RTOS target to break circular dependencies
│   ├── FreeRTOSConfig.h   # [Added Day 11] Nuclear macro definitions for SMP & Timers
│   ├── app
│   ├── common
│   ├── hal
│   └── main.c             # [Modified Day 11] RTOS Task definitions
└── test
    └── CMakeLists.txt

## 6. Development Workflow Rules
* **New Feature:** Create Source (`src/`) -> Create Test (`test/`) -> Update `test/CMakeLists.txt`.
* **Mocking Rule:** Testing App? Link `mock_hal_xxx.c`. Testing HAL? Include `test/mock` headers.
* **Commit Rule:** Never commit the `build_rp2350` folder or entire embedded open-source repositories (use `.gitignore`).

---

## 7. Next Steps (Phase 3: Task Design & RMS)
**Goal:** Wrap application logic into standard RTOS tasks and assign scientifically proven priorities.

**✅ Tasks Completed (Day 11 - OS Bring-up):**
* ✅ Integrated FreeRTOS v11.1.0 and broken Pico SDK 2.0 circular dependency.
* ✅ Enabled SMP (`configNUM_CORES=2`) and configured hardware porting layers.
* ✅ Verified Dynamic Load Balancing (Core 0/1 context switching) with `vTaskDelay`.
* ✅ Successfully initialized CYW43 Wi-Fi driver under the RTOS scheduler.

**Tasks (Day 12):**
1.  **Task Encapsulation:** Move OLED UI (`app_display`) and Boot Count (`app_storage`) logic into dedicated `vTask...` functions.
2.  **Rate Monotonic Scheduling (RMS):** Assign task priorities based on mathematical frequency (shorter period = higher priority).
3.  **Resource Protection:** Introduce Mutexes to protect shared hardware buses (I2C) from concurrent core access.

---

## 8. 🛠 Current Technical Debt (技術債與重構追蹤)

### Resolved (Day 11 Focus)
* ✅ **[Fixed] CMake Circular Dependency:** The official `pico_freertos` target causes a deadlock. Resolved by manually defining the `FreeRTOS-Kernel` target, extracting `tasks.c`/`port.c`, and forcefully injecting `target_include_directories`.
* ✅ **[Fixed] SMP Implicit Declaration:** `port.c` failed to compile due to missing `xTimerPendFunctionCallFromISR`. Resolved by enabling `configUSE_TIMERS 1` and `INCLUDE_xTimerPendFunctionCall 1` for cross-core FIFO messaging.

### Pending (Phase 3 & 4 Focus)
* ⚠️ **[High] CI/CD & Unit Test Breakage:** Including `"FreeRTOS.h"` in source files breaks the x86 Docker unit tests. We must implement RTOS API mocking before re-enabling local Git Hooks and resolving the GitHub Actions failure.
* ⚠️ **[High] Thread Safety (I2C/