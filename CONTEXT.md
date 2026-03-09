# Context: Pico 2 Automotive Firmware Project

## 1. Project Overview
* **Project Name:** pico2_automotive_firmware
* **Target Hardware:** Raspberry Pi Pico 2 W (RP2350 + CYW43) + SSD1306 OLED
* **Language:** C (Standard C99/C11)
* **Build System:** CMake + Docker (Standardized Build Env)
* **Testing:** Unity Framework (Unit Test) + Saleae/Oscilloscope (Physical Test)
* **Current Phase:** Phase 3 - RTOS Architecture & Safety (Completed Day 16, Starting Day 17)

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
* **Role:** Business Logic (e.g., `app_display`, `app_storage`, `app_button`, `app_fsm`, `app_sync`).
* **Responsibilities:**
    * Encapsulated into standard FreeRTOS task functions (`vAppDisplayTask`, etc.).
    * Utilizes absolute timing (`vTaskDelayUntil`) for Hard Real-time RMS execution.
    * Utilizes Deferred Interrupt Processing (`vAppButtonTask`) for software debouncing without blocking the CPU.
    * Executes a decoupled, Table-Driven Finite State Machine (`app_fsm`) driven by an RTOS message queue.
    * Centralizes cross-task synchronization via `app_sync` using FreeRTOS Event Groups.
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

**✅ Tasks Completed (Day 15 & 16 - FSM & Synchronization):**
* ✅ Implemented an $O(1)$ Table-Driven Finite State Machine utilizing a 2D array of Function Pointers.
* ✅ Decoupled the FSM engine into a dedicated FreeRTOS Task (`vAppFsmTask`) driven by a thread-safe Event Queue.
* ✅ **[Day 16]** Implemented the `app_sync` module using FreeRTOS Event Groups to create a thread-safe Boot Synchronization Barrier.
* ✅ **[Day 16]** Eliminated `while(!ready)` polling, reducing CPU load to 0% (Blocked state) during the boot waiting period.
* ✅ **[Day 16]** Integrated AND Logic (all ready) with OR Logic (any error) to realize a Fast-Fail mechanism, successfully validating that the FSM transitions to `FAULT` safely upon a 3000ms timeout.

**Tasks (Day 17 - System Crash Analysis Focus):**
1. **HardFault Handler:** Implement an ARM Cortex-M33 specific HardFault exception handler.
2. **Post-mortem Dump:** Extract and dump CPU Registers (PC, LR, MSP) to persistent storage (Flash) for offline crash analysis.

---

## 8. 🛠 Current Technical Debt

### Resolved (Day 12, 13, 14 Focus)
* ✅ **[Fixed] ISR to Task Communication:** Removed unsafe `printf` from ISR. Implemented FreeRTOS Direct Task Notification.
* ✅ **[Fixed] Thread Safety (I2C/SMP):** Implemented Mutex inside `hal_i2c`. Verified thread-safe execution without deadlocks.
* ✅ **[Fixed] CMake Dependency Graph:** Properly enforced `FreeRTOS-Kernel` linkages, solving `implicit declaration` errors.

### Pending (Phase 3 & 4 Focus)
* ⚠️ **[Low] CPU Starvation on Boot (Day 16 Identified):** Day 14 Priority Lab tasks (`MP_1`, `MP_2`) and CYW43 initialization heavily consume CPU during startup. This can starve the `vMonitorTask`, causing it to fail to send all Ready Bits within the 3000ms timeout and triggering a Fast-Fail. Future integration requires tuning task priorities or disabling lab code.
* ⚠️ **[Medium] ISR to FSM Communication:** `app_fsm_send_event()` currently only supports Task-level context via `xQueueSend`. If hardware interrupts (e.g., GPIO EXTI) need to directly trigger FSM state changes in the future, a dedicated `app_fsm_send_event_from_isr()` utilizing `xQueueSendFromISR` must be introduced to avoid RTOS assertion failures.
* ⚠️ **[Medium] Lab Code in Production:** The `hal_i2c_lab_simulate_long_transfer` API is currently compiled into the HAL layer. Needs to be stripped out via CMake `target_compile_definitions` in Phase 4 before final integration.
* ⚠️ **[High] CI/CD & Unit Test Breakage:** x86 Docker unit tests still cannot mock FreeRTOS APIs properly. Need to implement `mock_freertos.h` for GitHub Actions to pass.