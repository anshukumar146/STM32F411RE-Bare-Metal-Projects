# ⏱️ STM32 NUCLEO-F411RE — Bare-Metal TIM2 Timer Interrupt LED Toggle

**Level:** Bare-Metal Intermediate

A register-level STM32F411RE project that toggles an LED every second using **TIM2**, a hardware timer peripheral — generating a periodic, hardware-timed interrupt instead of relying on a blocking software delay loop. Written entirely in bare-metal C with no HAL, CubeMX-generated code, or middleware.

![STM32](https://img.shields.io/badge/MCU-STM32F411RE-03234B?style=for-the-badge&logo=stmicroelectronics&logoColor=white)
![Language](https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c&logoColor=white)
![IDE](https://img.shields.io/badge/IDE-STM32CubeIDE-blue?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Active-brightgreen?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)

---

## 📖 Project Overview

**Primary Objective**
Develop a bare-metal timer interrupt application on the STM32F411RE using TIM2, without relying on HAL or external libraries.

**Secondary Objectives**
- Understand how STM32 timers work internally
- Learn the relationship between the system clock, prescaler, counter, and auto-reload register
- Generate periodic interrupts using TIM2
- Control GPIO using interrupt-driven programming
- Learn the complete interrupt flow from the timer peripheral to the CPU
- Replace software delay loops with hardware timer interrupts

**Problem Statement**
Software delay loops (`for(volatile int i=0; i<N; i++);`) are inaccurate and waste CPU time, since the processor sits busy the entire time it's "waiting." This project's objective was to design a timer-based interrupt system that generates a periodic, hardware-timed interrupt every 1 second, executes its response automatically, eliminates blocking delay loops and allows timer hardware to determine when the next action occurs, and demonstrates event-driven programming instead of delay-based programming — building on the EXTI-based interrupt work in `03_Interrupt_LED_Toggle`, but replacing a hardware *event* (button press) with a hardware *timer* as the interrupt source.

**Real-World Applications**
Timer interrupts are foundational in real embedded systems — used for precise timekeeping, PWM generation, sensor polling at fixed intervals, communication protocol timing, watchdog refreshing, and any task that needs to happen "every N milliseconds" without burning CPU cycles on a busy-wait.

**Target Users**
- Embedded systems students learning STM32 timer peripherals at the register level
- Developers transitioning from software delay loops to hardware timer-driven design
- Anyone using a NUCLEO-F411RE board wanting a minimal, from-scratch TIM2 interrupt example without HAL

---

## ✨ Features

✅ Configures TIM2 as a free-running hardware timer generating an interrupt every 1 second
✅ Uses the prescaler (PSC) and auto-reload register (ARR) to derive predictable, hardware-timed intervals from the system clock
✅ Forces an immediate register update via the Event Generation Register (EGR) at initialization
✅ Enables the timer's update interrupt and routes it through the NVIC
✅ Toggles an LED (PC3) directly from within the `TIM2_IRQHandler()` ISR
✅ `main()` stays idle in an empty loop — all periodic LED behavior lives in the ISR
✅ Fully register-level — no HAL, no CubeMX-generated peripheral code

---

## 🛠️ Technologies Used

| Category | Details |
|---|---|
| **Language** | C (bare-metal, register-level) |
| **MCU** | STM32F411RETx (Cortex-M4, 512KB Flash, 128KB RAM) |
| **Board** | STM32 NUCLEO-F411RE |
| **IDE** | STM32CubeIDE |
| **Toolchain** | GNU ARM GCC (`arm-none-eabi-`) |
| **Debugger/Programmer** | ST-Link (via STM32CubeIDE's built-in GDB server) |
| **Build System** | Eclipse CDT Managed Build (STM32CubeIDE project) |
| **Version Control** | Git & GitHub |

---

## 🔌 Hardware

| Component | Pin | Configuration |
|---|---|---|
| LED | PC3 | Output — toggled every 1 second by `TIM2_IRQHandler()` |
| *(configured but unused)* | PC0 | Input, internal pull-up |
| *(configured but unused)* | PC4 | Output |

> **Note on unused pins:** `GPIO_Init()` configures PC0 as an input with a pull-up and PC4 as an output, mirroring the pin setup from the earlier `03_Interrupt_LED_Toggle` project — but neither pin is actually read or driven anywhere in this program. Only PC3 is toggled, inside the timer ISR. This looks like leftover configuration carried over from the previous project's `GPIO_Init()` rather than something required for TIM2 functionality. It's harmless (won't cause incorrect behavior) but isn't necessary for this project's stated goal.

---

## 📂 Folder Structure

```
04_TIM2_Interrupt_LED_Toggle/
│
├── .project                                              # Eclipse/STM32CubeIDE project descriptor
├── .cproject                                              # Build configuration (Debug/Release, compiler/linker options)
├── 04_TIM2_Interrupt_LED_Toggle Debug.launch             # Debug launch configuration (ST-Link settings)
├── STM32F411RETX_FLASH.ld                                # Linker script — runs code from Flash
├── STM32F411RETX_RAM.ld                                  # Linker script — runs code from RAM (debug-in-RAM)
│
├── Src/
│   ├── main.c                                              # GPIO + TIM2 configuration and interrupt handler
│   ├── syscalls.c                                          # Newlib syscall stubs (auto-generated)
│   └── sysmem.c                                            # Heap management for malloc/newlib (auto-generated)
│
└── Startup/
    └── startup_stm32f411retx.s                              # Reset handler, vector table (includes TIM2_IRQHandler entry)
```

---

## ⚙️ How It Works

1. **GPIO Setup (`GPIO_Init`)** — Enables the GPIOC peripheral clock and configures PC3 as a general-purpose output for the LED (PC0 and PC4 are also configured but unused — see note above).
2. **Timer Setup (`TIM2_Init`)** — Enables the TIM2 peripheral clock via `RCC_APB1ENR`, sets the prescaler (`TIM2_PSC`) and auto-reload value (`TIM2_ARR`) to produce a 1-second period, forces those values to load immediately via `TIM2_EGR`, clears any stale update flag, enables the update interrupt in `TIM2_DIER`, enables TIM2's IRQ line (IRQ28) in the NVIC, and finally starts the counter via `TIM2_CR1`.
3. **Idle Main Loop** — `main()` calls `GPIO_Init()` and `TIM2_Init()` once, then enters an empty `while(1)` loop — the CPU does nothing else; all periodic behavior is driven entirely by the timer hardware.
4. **Interrupt Service Routine (`TIM2_IRQHandler`)** — Once per second, when the counter reaches `ARR` and rolls over, hardware automatically sets the update interrupt flag and the CPU jumps to this handler. It clears the pending flag (`TIM2_SR`), toggles the LED on PC3 via XOR on `GPIOC_ODR`, and returns — resuming whatever `main()` was doing.

---

## 🔍 Code Explanation

**`Src/main.c`**

| Function | Purpose |
|---|---|
| `GPIO_Init()` | Enables the GPIOC clock; configures PC3 as output (LED); also configures PC0 (input, pull-up) and PC4 (output), though neither is used elsewhere in this program. |
| `TIM2_Init()` | Enables the TIM2 clock; sets `PSC`/`ARR` for a 1-second period; forces an immediate register update via `EGR`; clears the update flag; enables the update interrupt; enables TIM2's NVIC line; starts the counter. |
| `TIM2_IRQHandler()` | The interrupt service routine. Clears the TIM2 update interrupt flag (`UIF`), then toggles PC3 (the LED). |
| `main()` | Calls `GPIO_Init()` and `TIM2_Init()` once at startup, then idles in an empty `while(1)` loop, relying entirely on the timer interrupt to drive LED behavior. |

**Register Table**

| Register | Bit | Purpose |
|---|---|---|
| `RCC_AHB1ENR` | 2 | GPIOC Clock Enable |
| `RCC_APB1ENR` | 0 | TIM2 Clock Enable |
| `GPIOC_MODER` | 6–7 | PC3 Mode = General-Purpose Output |
| `GPIOC_PUPDR` | 0–1 | PC0 Pull Configuration = Pull-Up |
| `TIM2_PSC` | 0–15 | Prescaler Value = 15999 |
| `TIM2_ARR` | 0–15 | Auto-Reload Value = 999 |
| `TIM2_EGR` | 0 | Force Update Event (UG) |
| `TIM2_SR` | 0 | Update Interrupt Flag (UIF) |
| `TIM2_DIER` | 0 | Update Interrupt Enable (UIE) |
| `TIM2_CR1` | 0 | Counter Enable (CEN) |
| `NVIC_ISER0` | 28 | Enable IRQ28 (TIM2 global interrupt) |

**Timing calculation (as implemented):**
```
Timer Clock (APB1, default HSI)  = 16 MHz
Counter Frequency = Timer Clock / (PSC + 1) = 16,000,000 / 16,000 = 1,000 Hz  → 1 ms per tick
Interrupt Period  = (ARR + 1) × tick period = 1,000 × 1 ms = 1,000 ms = 1 second
```
This calculation assumes the MCU is still running on its default post-reset clock configuration — **HSI at 16 MHz**, with no PLL or `SystemClock_Config()` reconfiguring it. This project doesn't define its own `SystemInit()`, so the weak stub in the startup file is used, meaning the clock is never reconfigured and stays at the default 16 MHz HSI source. If a future version of this project (or a shared `system_stm32f4xx.c`) changes the system clock, `PSC`/`ARR` would need to be recalculated to still land on exactly 1 second.

---

## 🧠 Key Concepts Mastered

**1. Peripheral Clock Enable**
Enabling a peripheral's clock is what allows it to receive clock pulses from the RCC clock tree. Without it: the timer cannot count, its registers cannot operate, and interrupts never occur.

**2. Prescaler (PSC) — "speed of the runner"**
Controls how fast the counter increments:
```
Counter Frequency = Timer Clock / (PSC + 1)
```

**3. Auto-Reload Register (ARR) — "distance of the race"**
Controls how many counter increments occur before an Update Event fires.

**4. PSC vs. ARR**
PSC controls *speed*; ARR controls the *count limit*. Together, they determine the interrupt period.

**5. Update Event vs. Update Interrupt**
An Update Event is an internal hardware event — not the interrupt itself — generated when `CNT` reaches `ARR`, or when software forces it via `UG`. That event sets the `UIF` flag, and *only if* the update interrupt is enabled (`DIER`'s `UIE` bit) does an actual interrupt request follow:
```
Update Event → UIF = 1 → Interrupt request (if enabled)
```

**6. Why EGR / UG Matters**
Writing `UG` in `EGR` forces an Update Event, ensuring the newly written prescaler value is transferred to the active timer logic immediately rather than waiting for a natural update event.

**7. Buffered Registers**
`PSC` is buffered: writing `TIM2_PSC = value` doesn't take effect immediately. Instead, it flows through a buffer and only becomes the active prescaler value at the next Update Event (or when forced via `EGR`):
```
CPU → Buffer → Update Event → Active Prescaler
```

**8. Full Interrupt Flow**
```
Timer Clock → PSC → CNT → ARR reached → Update Event → UIF = 1
→ TIM2_DIER (UIE) → NVIC → CPU → TIM2_IRQHandler() → Clear UIF → Toggle LED
```

**9. TIM2 vs. EXTI**
| | TIM2 | EXTI |
|---|---|---|
| Trigger type | Time-based interrupt | Event-based interrupt |
| Pending flag clear | `UIF` — write **0** | `PR` — write **1** |

**10. Interrupt-Driven vs. Delay-Based Programming**
Instead of:
```c
while(1) {
    delay();
    LED ^= 1;
}
```
this project demonstrates the timer-interrupt equivalent — the hardware itself triggers the LED toggle on schedule, with the CPU not blocked in a busy-wait delay loop in between, which is significantly more efficient and provides predictable, hardware-timed periodicity.

---

## 🐛 Problems Faced During Development

Based on the development notes for this project, the following areas required the most work to understand correctly:

- Understanding the actual purpose of the prescaler (PSC)
- Understanding what ARR really controls
- Confusion between an Update Event and the interrupt it can trigger
- Understanding why `EGR`/`UG` is required during initialization
- Understanding why `PSC` is a buffered register
- Understanding why `ARR` and `PSC` values are set as `desired_value - 1`
- Distinguishing `UIF` (the flag) from the Update Event (the hardware occurrence) that sets it
- Understanding the difference between `DIER` (interrupt enable) and `SR` (status/pending flags)
- Confusing the EXTI pending-flag clearing convention with TIM2's (write 1 vs. write 0)
- Understanding the NVIC's role at the CPU level versus the timer peripheral's role
- Choosing the correct order of initialization steps

---

## 💡 Skills Gained

- Reading the STM32 Reference Manual instead of copying pre-written code
- Register-level timer programming
- Interrupt configuration and debugging
- Calculating PSC/ARR values from a target interrupt period instead of memorizing "magic numbers"
- Event-driven firmware design
- Bare-metal STM32 development
- Hardware/software integration

---

## 💻 Installation

**1. Clone the repository**
```bash
git clone https://github.com/anshukumar146/github.git
cd github/NUCLEOF411RE/04_TIM2_Interrupt_LED_Toggle
```

**2. Install STM32CubeIDE**
Download and install from [st.com/en/development-tools/stm32cubeide.html](https://www.st.com/en/development-tools/stm32cubeide.html) if not already installed.

**3. Import the project**
- Open STM32CubeIDE
- Go to **File → Open Projects from File System...**
- Select the `04_TIM2_Interrupt_LED_Toggle` folder
- Click **Finish** to import

---

## ▶️ Running the Project

1. Wire an LED (with a current-limiting resistor) to **PC3**. No button or other input hardware is required for this project's core functionality.
2. Connect the NUCLEO-F411RE board to your computer via USB.
3. In STM32CubeIDE, select the project in the Project Explorer.
4. Click **Build** to compile.
5. Click **Run → Debug** (using the `04_TIM2_Interrupt_LED_Toggle Debug.launch` configuration) to flash the firmware via ST-Link.
6. Once flashed, the program runs automatically — the LED should toggle once every second, indefinitely, with no button interaction needed.

---

## 🧑‍💻 Usage

- **No user interaction required** — once flashed, the LED on PC3 toggles automatically every 1 second, driven entirely by the TIM2 hardware interrupt.
- `main()` performs no application work while waiting for the next interrupt; the CPU continuously executes the empty loop between timer interrupts, rather than busy-waiting in a delay loop.


---

## 🖼️ Schematic diagram
<img width="466" height="477" alt="Screenshot 2026-08-19 214732" src="https://github.com/user-attachments/assets/fd0592a1-7f90-472d-90c0-829f491c98ea" />


---

## 🖼️ Screenshots

## Output Screenshot
<img width="900" height="700" alt="Timer_Scheduler" src="https://github.com/user-attachments/assets/73ee81e1-219f-46cc-8055-5ac65d7384a1" />


---

## 🎥 Demo Video




https://github.com/user-attachments/assets/7798ac5a-d917-464e-9d99-9275958049b6




---

## 📊 Results

Successfully implemented a bare-metal TIM2 interrupt application that:
- Initializes TIM2 entirely at the register level
- Generates a periodic, hardware-timed 1-second interrupt (based on the default 16 MHz HSI clock)
- Executes its ISR automatically, with no CPU polling involved
- Clears the interrupt flag correctly, using the write-0 convention specific to TIM2's `SR` register
- Toggles the LED on PC3 every second, indefinitely
- Uses no HAL, LL, or middleware of any kind

---

## 🚀 Future Improvements

- Remove the unused PC0 (input/pull-up) and PC4 (output) configuration in `GPIO_Init()` if they're not needed for a future feature, to keep the code minimal and avoid confusion for readers
- Explicitly configure and document the system clock (e.g., via a real `SystemClock_Config()`) rather than relying on the implicit default HSI 16 MHz, so the PSC/ARR timing math is self-documenting and robust to future changes
- Add a compile-time or runtime assertion/comment tying the PSC/ARR values directly to the assumed clock speed, to prevent silent timing drift if the clock source changes later
- Extend the project to drive multiple timers (e.g., TIM2 + TIM3) to demonstrate multiple independent periodic interrupts
- Combine this project with `03_Interrupt_LED_Toggle` to show a button (EXTI) and a timer (TIM2) both driving independent LEDs simultaneously, directly contrasting event-based vs. time-based interrupts on the same board

---

## 🎓 Learning Outcomes

By studying and building this project, you will learn:

- How peripheral clock enables gate whether a peripheral can operate at all
- The relationship between the timer clock, prescaler, counter, and auto-reload register
- How to calculate PSC/ARR values to hit a target interrupt period
- Why buffered registers like `PSC` require an `EGR`/`UG` force-update to take effect immediately, rather than waiting for a natural update event
- The complete interrupt chain: timer clock → counter → Update Event → UIF → DIER → NVIC → CPU → ISR
- Why the Update Event and the Update Interrupt are two distinct concepts, not the same thing
- Why `TIM2_SR`'s UIF is cleared by writing 0, unlike EXTI's `PR`, which is cleared by writing 1
- How to replace inaccurate, CPU-blocking software delay loops with predictable, non-blocking hardware timer interrupts

**By completing this project, you should be able to confidently explain:**
- How the timer's prescaler and auto-reload register combine to set an interrupt period
- Why enabling a peripheral clock is a prerequisite for using any STM32 peripheral
- The difference between an Update Event and an Update Interrupt
- Why `EGR`/`UG` is needed during initialization
- Why `PSC` is a buffered register and what that means in practice
- The complete interrupt flow from the timer peripheral through to `TIM2_IRQHandler()`
- Why TIM2's pending flag is cleared differently than EXTI's

---

## 🧩 Skills Demonstrated

- Bare-Metal Embedded C Programming
- ARM Cortex-M Timer Peripheral Configuration (TIM2)
- Interrupt Handling (NVIC, ISR design)
- STM32 Peripheral Clock & Register-Level Configuration
- Precise Timing Calculation (Prescaler/Auto-Reload Math)
- Event-Driven Firmware Design
- STM32CubeIDE Project Setup & ST-Link Debugging
- Embedded Systems Documentation

---

## 👤 Author

**Name:** Anshu Kumar
**GitHub:** [@anshukumar146](https://github.com/anshukumar146)
**LinkedIn:** _(add your LinkedIn URL here)_
**Email:** _(add your email here)_

---

## 📜 License

This project is licensed under the **MIT License**. Note that `startup_stm32f411retx.s` and the `.ld` linker scripts are STM32CubeIDE/STMicroelectronics-generated boilerplate, licensed under terms provided by STMicroelectronics (see file headers).

```
MIT License

Copyright (c) 2026 Anshu Kumar

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## 🙏 Acknowledgements

- STMicroelectronics for STM32CubeIDE and the auto-generated project scaffolding (linker scripts, startup code)
- The STM32F411xC/E Reference Manual for TIM2, RCC, and NVIC register documentation
- The embedded systems community for resources on bare-metal ARM Cortex-M timer programming

---

<div align="center">

⭐ If you found this repository helpful, consider giving it a star!

</div>
