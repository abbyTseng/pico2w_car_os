# Context: Pico 2 Automotive Firmware Project

## 1. Project Overview
* **Project Name:** pico2_automotive_firmware
* **Target Hardware:** Raspberry Pi Pico 2 W (RP2350 + CYW43) + SSD1306 OLED
* **Language:** C (Standard C99/C11)
* **Build System:** CMake + Docker (Standardized Build Env)
* **Testing:** Unity Framework (Unit Test) + Saleae/Oscilloscope (Physical Test)
* **Current Phase:** Phase 3 - IPC & Interrupt Decoupling (Starting Day 13)

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
* **Role:** Business Logic (e.g., `app_display`, `app_storage`).
* **Responsibilities:**
    * Encapsulated into standard FreeRTOS task functions (`vAppDisplayTask`, etc.).
    * Utilizes absolute timing (`vTaskDelayUntil`) for Hard Real-time RMS execution.
    * Calls HAL interfaces using `common_status_t`.
* **Constraint:** Hardware-agnostic. Should run on any MCU if HAL is provided. FreeRTOS API usage is restricted to `.c` files, never in `.h` headers.

### C. HAL Layer (`src/hal/`)
* **Role:** Hardware Abstraction.
* **Key Modules:**
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

## 7. Next Steps (Phase 3: IPC & Interrupt Decoupling)
**✅ Tasks Completed (Day 12 - Task Design & RMS):**
* ✅ Refactored `main.c` to be a pure OS scheduler starter, fixing Leaky Abstractions (`pico/stdlib.h` removed).
* ✅ Implemented absolute timing using `vTaskDelayUntil` based on RMS (100ms/500ms/1000ms), eliminating Jitter.
* ✅ Implemented FreeRTOS Mutex (`xSemaphoreTake` with timeout) in `hal_i2c.c` to protect hardware resources from SMP concurrent access.
* ✅ Tuned Task Stack sizes to 1024 Words to prevent Silent Crashes during CYW43 initialization.
* ✅ Demonstrated "Graceful Degradation" (Fault Isolation) by suspending the OLED task upon hardware I2C NACK without crashing the system.

**Tasks (Day 13):**
1. **IPC (Inter-Process Communication):** Introduce FreeRTOS Queues or Direct Task Notifications to pass data safely between tasks.
2. **Interrupt Decoupling:** Refactor GPIO callbacks (`on_button_press`). Move logic out of ISR context into a deferred handler task to ensure system responsiveness.

---

## 8. 🛠 Current Technical Debt

### Resolved (Day 12 Focus)
* ✅ **[Fixed] Thread Safety (I2C/SMP):** Implemented Mutex inside `hal_i2c`. Verified thread-safe execution without deadlocks.
* ✅ **[Fixed] CMake Dependency Graph:** Properly enforced `FreeRTOS-Kernel` and `pico_stdio_usb` linkages at the HAL/App levels, solving `implicit declaration` errors.

### Pending (Phase 3 & 4 Focus)
* ⚠️ **[High] CI/CD & Unit Test Breakage:** x86 Docker unit tests still cannot mock FreeRTOS APIs properly. Need to implement `mock_freertos.h` for GitHub Actions to pass.
* ⚠️ **[Medium] ISR to Task Communication:** GPIO callbacks currently just ignore parameters or use `printf` (unsafe in ISR). Needs RTOS Queue implementation.