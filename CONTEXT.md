# Context: Pico 2 Automotive Firmware Project

## 1. Project Overview
* **Project Name:** pico2_automotive_firmware
* **Target Hardware:** Raspberry Pi Pico 2 W (RP2350 + CYW43) + SSD1306 OLED
* **Language:** C (Standard C99/C11)
* **Build System:** CMake + Docker (Standardized Build Env)
* **Testing:** Unity Framework (Unit Test) + Saleae/Oscilloscope (Physical Test)
* **Current Phase:** Phase 3 - RTOS Architecture & Safety (Completed Day 13, Starting Day 14)

## 2. Architecture & Design Patterns (Layered Architecture)
We have refactored the system into a strict 3-layer architecture to ensure decoupling and testability.

### A. Main Layer (`src/main.c`)
* **Role:** System Scheduler & Configurator.
* **Responsibilities:**
    * Initializes System HAL (`hal_init_system`, `hal_init_is_usb_connected`).
    * Injects dependencies and creates FreeRTOS Tasks (`vTaskCreate`).
    * Starts FreeRTOS Scheduler (`vTaskStartScheduler`).
* **Constraint:** **NO** direct hardware manipulation logic or original SDK includes (like `pico/stdlib.h`) allowed here. Super loop replaced by RTOS Tasks.

### B. App Layer (`src/app/`)
* **Role:** Business Logic (e.g., `app_display`, `app_storage`, `app_button`).
* **Responsibilities:**
    * Encapsulated into standard FreeRTOS task functions (`vAppDisplayTask`, etc.).
    * Utilizes absolute timing (`vTaskDelayUntil`) for Hard Real-time RMS execution.
    * Utilizes Deferred Interrupt Processing (`vAppButtonTask`) for software debouncing without blocking the CPU.
    * Calls HAL interfaces using `common_status_t`.
* **Constraint:** Hardware-agnostic. Should run on any MCU if HAL is provided. FreeRTOS API usage is restricted to `.c` files, never in `.h` headers.

### C. HAL Layer (`src/hal/`)
* **Role:** Hardware Abstraction.
* **Key Modules:**
    * **`hal_gpio`:** Implements minimal ISR routines. Exposes strict `hal_gpio_isr_callback_t` interface to prevent accidental blocking API usage from App layer.
    * **`hal_i2c`:** Protected by FreeRTOS Mutex (`xSemaphoreCreateMutex`) for SMP thread safety. Implements "9-Clock Recovery" within the locked state.
    * **`hal_init`:** Wraps SDK specific modules (e.g., `pico_stdio_usb`) to prevent leaky abstractions.
    * **`hal_led`:** Abstracts Pico 2W's CYW43 wireless LED control.
    * **`hal_storage`:** Abstraction for LittleFS on Flash with XIP protection.

---

## 3. CI/CD Architecture (Dual-Track & Shift-Left Strategy)

*Note: Currently in OS Bring-up transition. CI/CD pipelines are expected to fail until FreeRTOS mocking is implemented.*

### A. Local Guardrail (The "Hook")
* **Trigger:** `git commit` via `.git/hooks/pre-commit` (Implementation delayed to Phase 4).
* **Strategy:** "Shift-Left Testing". Includes strict compiler checks (`-Werror`).
* **Current Status:** Temporarily disabled to allow committing OS integration code without triggering false-positive test failures.

### B. Cloud Factory (GitHub Actions)
* **Trigger:** `git push` (`.github/workflows/main.yml`)
* **Actions:** Lint (Cppcheck), Test (CTest), Build (`.uf2`).
* **Current Status:** Red (Failed) ❌. This is an accepted technical debt while we migrate the test suite to recognize FreeRTOS headers.

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

---

## 5. Key Directory Structure
.
├── external
│   ├── FreeRTOS-Kernel    # v11.1.0 SMP Kernel
│   └── littlefs           
├── src
│   ├── CMakeLists.txt     # Top-to-bottom dependency graph (OS -> HAL -> App)
│   ├── FreeRTOSConfig.h   # Core config: configNUM_CORES=2, configUSE_MUTEXES=1
│   ├── app
│   ├── common
│   ├── hal
│   └── main.c             # Pure OS scheduler starter
└── test

---

## 7. Next Steps (Phase 3: RTOS Architecture & Safety)

**✅ Tasks Completed (Day 13 - IPC Patterns & ISR Decoupling):**
* ✅ Implemented Direct Task Notification (`vTaskNotifyGiveFromISR`) to decouple GPIO interrupts from application logic.
* ✅ Built a Deferred Handler Task (`vAppButtonTask`) with a 50ms timestamp-based software debounce.
* ✅ Achieved 0-byte dynamic memory overhead for IPC and guaranteed ISR execution time < 5μs.
* ✅ Eliminated `printf` and blocking operations from ISR, removing the risk of CPU starvation and SMP deadlocks.

**Tasks (Day 14 - Mutex & Priority Inversion Focus):**
1. **Priority Inversion Simulation:** Create a deliberate scenario where a low-priority task holds a resource (Mutex), a high-priority task waits for it, and a medium-priority task preempts the low-priority one.
2. **Priority Inheritance Validation:** Demonstrate how FreeRTOS automatically promotes the low-priority task to resolve the deadlock, analyzing the RTOS trace or logs.

---

## 8. 🛠 Current Technical Debt

### Resolved (Day 12 & Day 13 Focus)
* ✅ **[Fixed] ISR to Task Communication:** Removed unsafe `printf` from ISR. Implemented FreeRTOS Direct Task Notification for safe, zero-overhead interrupt deferred processing.
* ✅ **[Fixed] Thread Safety (I2C/SMP):** Implemented Mutex inside `hal_i2c`. Verified thread-safe execution without deadlocks.
* ✅ **[Fixed] CMake Dependency Graph:** Properly enforced `FreeRTOS-Kernel` and `pico_stdio_usb` linkages at the HAL/App levels, solving `implicit declaration` errors.

### Pending (Phase 3 & 4 Focus)
* ⚠️ **[High] CI/CD & Unit Test Breakage:** x86 Docker unit tests still cannot mock FreeRTOS APIs properly. Need to implement `mock_freertos.h` for GitHub Actions to pass.