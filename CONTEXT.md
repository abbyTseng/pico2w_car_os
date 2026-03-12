# Context: Pico 2 Automotive Firmware Project

## 1. Project Overview
* **Project Name:** pico2_automotive_firmware
* **Target Hardware:** Raspberry Pi Pico 2 W (RP2350 + CYW43) + SSD1306 OLED
* **Language:** C (Standard C99/C11)
* **Build System:** CMake + Docker (Standardized Build Env)
* **Testing:** Unity Framework (Unit Test) + Saleae/Oscilloscope (Physical Test)
* **Current Phase:** Phase 5 - Diagnostics & UDS (Phase 4 Security, Reliability & DevOps Completed)

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
* **Role:** Business Logic (e.g., `app_display`, `app_storage`, `app_button`, `app_fsm`, `app_sync`, `app_monitor`, `app_sensor`).
* **Responsibilities:**
    * Encapsulated into standard FreeRTOS task functions.
    * Utilizes absolute timing (`vTaskDelayUntil`) for Hard Real-time RMS execution.
    * Utilizes Deferred Interrupt Processing (`vAppButtonTask`) for software debouncing.
    * Executes a decoupled, Table-Driven Finite State Machine (`app_fsm`).
    * Centralizes cross-task synchronization via `app_sync` using FreeRTOS Event Groups.
    * Implements a Software Window Watchdog (SW-WWDT) via `app_monitor`.
    * Implements robust Queue Overrun Handling (`app_sensor`) to prevent dynamic memory fragmentation.
* **Constraint:** Hardware-agnostic. FreeRTOS API usage is restricted to `.c` files, never in `.h` headers.

### C. HAL Layer (`src/hal/`)
* **Role:** Hardware Abstraction.
* **Key Modules:**
    * **`hal_gpio`:** Implements minimal ISR routines. Exposes strict `hal_gpio_isr_callback_t`.
    * **`hal_i2c`:** Protected by FreeRTOS Mutex (`xSemaphoreCreateMutex`). Implements "9-Clock Recovery".
    * **`hal_init`:** Wraps SDK specific modules to prevent leaky abstractions.
    * **`hal_led`:** Abstracts Pico 2W's CYW43 wireless LED control.
    * **`hal_storage`:** Abstraction for LittleFS on Flash with XIP protection.
    * **`hal_fault`:** Implements Cortex-M33 HardFault interception via naked assembly.
    * **`hal_wdt`:** Hardware Watchdog abstraction (1500ms timeout).

---

## 3. CI/CD Architecture (Dual-Track & Shift-Left Strategy)

*Note: Infrastructure is fully stabilized. Both Local and Cloud gates are strictly enforced.*

### A. Local Guardrail (The "Hook")
* **Trigger:** `git commit` via `.pre-commit-config.yaml`.
* **Strategy:** "Shift-Left Testing" preventing bad code from entering Git history.
* **Pipeline:** 1. 🎨 `clang-format` 2. 🔍 `cppcheck` (MISRA) 3. 🐳 `docker-unit-test`.

### B. Cloud Factory (GitHub Actions)
* **Trigger:** `git push` (`.github/workflows/main.yml`)
* **Strategy:** "ISO 26262 Reproducible Build" via strict dependency vendoring.
* **Status:** Green (Passed) ✅.

---

## 4. Testing Strategy (Unity Framework)

We utilize a **Mirroring Directory Structure** to map `test/` 1:1 with `src/`, ensuring high maintainability.

### A. Mocking Strategy
* **Level 1: Component Isolation (Static Library Mocking)**
    * **Goal:** Prevent "Multiple Definition" Linker errors when mocking HAL/Common components.
    * **Method:** All mock implementations are compiled into a centralized `libtest_mocks.a` static library. The C linker selectively resolves only undefined symbols from this library.
* **Level 2: RTOS Time & State Manipulation**
    * **Goal:** Verify thread-safety, timing logic, and edge cases safely on x86.
    * **Method:** `mock_freertos.c` intercepts and bypasses native APIs. Features include:
        * **Time Travel:** Manipulating `xTaskGetTickCount` to test software debounce (`app_button`).
        * **Queue Simulation:** Forcing `errQUEUE_FULL` to test overrun handling (`app_sensor`).
        * **Barrier Interception:** Stateful `EventGroup` mock to test synchronous boot behaviors (`app_sync`).

---

## 5. Key Directory Structure
.
├── external
│   ├── FreeRTOS-Kernel    # v11.1.0 SMP Kernel (Vendored)
│   └── littlefs           # (Vendored)
├── src
│   ├── app                # Business Logic
│   ├── common             # Shared Types/Buffers
│   ├── hal                # Hardware Abstraction
│   └── main.c             # System Starter
└── test
    ├── CMakeLists.txt     # Test Build Graph (Macros + test_mocks.a)
    ├── app/               # Mirror of src/app/ (100% Covered)
    ├── common/            # Mirror of src/common/
    ├── hal/               # Mirror of src/hal/
    └── mock/              # Centralized Mocks (mock_freertos.c, etc.)

---

## 7. Next Steps (Phase 5 Transition)

**✅ Tasks Completed (Day 18 - App Layer Unit Testing & CI/CD):**
* ✅ **100% App Layer Test Coverage:** FSM, Sync, Watchdog, Button Debounce, Sensor Queue, Storage, Display.
* ✅ **Test Architecture Refactoring:** Implemented Mirroring Structure and Static Library Mocking.
* ✅ **Dual-Layer Watchdog:** Hardware WDT + Software Task Monitor (SW-WWDT).
* ✅ **Vendoring:** Absorbed FreeRTOS and littlefs for cloud reproducibility.

**🔜 Tasks Pending (Phase 5 - Diagnostics):**
1. **[Day 19]** Implement a robust DTC (Diagnostic Trouble Code) generation and logging mechanism in `app_storage` & `app_fsm`.
2. **[Day 20]** Implement UDS (Unified Diagnostic Services) basic protocol to read/clear DTCs via UART/USB.

---

## 8. 🛠 Current Technical Debt

### Resolved (Day 18 Focus)
* ✅ **[Fixed] Local Git Guardrails & CI/CD Breakage:** Full DevOps pipeline is now Green.
* ✅ **[Fixed] ISR to FSM Communication:** `app_fsm_send_event_from_isr()` using `xQueueSendFromISR` has been successfully implemented and unit-tested to support deferred hardware interrupts.
* ✅ **[Fixed] Linker Conflicts in Testing:** Resolved via `libtest_mocks.a`.

### Pending
* ⚠️ **[Medium] Lab Code in Production:** The `hal_i2c_lab_simulate_long_transfer` API is currently compiled into the HAL layer. Needs to be stripped out via CMake `target_compile_definitions` before final release.
* ⚠️ **[Medium] Crash Log Persistence (Day 17/19):** Currently, the `hal_fault` crash report is only stored in No-Init RAM (survives Warm Reset). It needs to be written to LittleFS in `hal_fault_check_and_log_crash()` to survive a Cold Boot.
* ⚠️ **[Low] FPU Extended Frame Handling (Day 17):** RP2350 has hardware FPU enabled. If a crash occurs while the FPU is in use (`EXC_RETURN` bit 4 is 0), the hardware pushes an extended frame (S0-S15). Needs dynamic offset adjustment for future robustness.