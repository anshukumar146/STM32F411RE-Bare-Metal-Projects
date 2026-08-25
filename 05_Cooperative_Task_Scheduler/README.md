# 🗓️ STM32 NUCLEO-F411RE — Bare-Metal Cooperative Task Scheduler

**Level:** Bare-Metal Advanced

A register-level STM32F411RE project that builds a small **cooperative task scheduler** on top of a single TIM2 hardware timer — running three independent periodic tasks (two LEDs and periodic button polling with falling-edge detection) from one 1 ms system tick, entirely without an RTOS. Written in bare-metal C with no HAL, CubeMX-generated code, or middleware.


![STM32](https://img.shields.io/badge/MCU-STM32F411RE-03234B?style=for-the-badge&logo=stmicroelectronics&logoColor=white)
![Language](https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c&logoColor=white)
![IDE](https://img.shields.io/badge/IDE-STM32CubeIDE-blue?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Active-brightgreen?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)

---

## 📖 Project Overview

**Main Idea**
Use TIM2 as a 1 ms system tick, and use software counters to schedule multiple independent tasks — the hardware timer provides the *time base*; `main()` executes the *tasks*.

**Objective**
This project moves beyond simply toggling one LED inside a TIM2 ISR (as in `04_TIM2_Interrupt_LED_Toggle`) and builds a small cooperative task scheduler. Specifically, the goals were to:
- Generate a regular 1 ms system tick using TIM2
- Maintain separate software timers for different tasks
- Run a Green LED task every 500 ms
- Run a Red LED task every 1000 ms
- Check a button every 10 ms
- Use ready flags to communicate between the ISR and `main()`
- Keep the ISR short
- Execute actual application tasks from `main()`
- Understand how a basic embedded scheduler works without an RTOS

**Real-World Applications**
Cooperative scheduling is a foundational pattern in real embedded firmware — used any time a single microcontroller needs to juggle multiple periodic responsibilities (sensor sampling, UI updates, communication polling, actuator control) without the overhead of a full RTOS. It's the conceptual stepping stone between "one interrupt, one job" and "many independent tasks, driven by one system tick."

**Target Users**
- Embedded systems students moving from single-task interrupt handling toward multi-task scheduling concepts
- Developers wanting a minimal, from-scratch example of a cooperative (non-preemptive) scheduler before adopting an RTOS
- Anyone using a NUCLEO-F411RE board wanting to see ISR-to-main communication patterns in practice

---

## ✨ Features

✅ Single TIM2 hardware timer provides a shared 1 ms system tick for the entire scheduler
✅ Three independent software timers (`green_timer`, `red_timer`, `button_timer`) track separate task periods from one time base
✅ Ready-flag pattern (`green_ready`, `red_ready`, `button_ready`) cleanly separates "time has passed" (ISR) from "task executed" (`main()`)
✅ Green LED task runs every 500 ms; Red LED task runs every 1000 ms; button check runs every 10 ms
✅ Proper falling-edge detection on the button — a detected falling edge produces one LED toggle, not a rapid re-toggle stream
✅ Deliberately short ISR — all it does is update timers and set flags; actual task logic lives entirely in `main()`
✅ Fully register-level — no HAL, no CubeMX-generated peripheral code, no RTOS

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

| Pin | Function |
|---|---|
| PC0 | Push button (internal pull-up) |
| PC1 | Third LED, toggled by the button task |
| PC3 | Green LED (500 ms task) |
| PC4 | Red LED (1000 ms task) |

**Button wiring (internal pull-up):**
```
3.3V ── Internal Pull-up ── PC0 ── Button ── GND

Released → PC0 reads 1
Pressed  → PC0 reads 0
```

**LED wiring:**
```
PC1 ── resistor ── LED       ── GND
PC3 ── resistor ── Green LED ── GND
PC4 ── resistor ── Red LED   ── GND
```

---

## 📂 Folder Structure

```
05_Cooperative_Task_Scheduler/
│
├── .project                                              # Eclipse/STM32CubeIDE project descriptor
├── .cproject                                              # Build configuration (Debug/Release, compiler/linker options)
├── 05_Cooperative_Task_Scheduler Debug.launch             # Debug launch configuration (ST-Link settings)
├── STM32F411RETX_FLASH.ld                                # Linker script — runs code from Flash
├── STM32F411RETX_RAM.ld                                  # Linker script — runs code from RAM (debug-in-RAM)
│
├── Src/
│   ├── main.c                                              # GPIO + TIM2 config, scheduler ISR, and task loop
│   ├── syscalls.c                                          # Newlib syscall stubs (auto-generated)
│   └── sysmem.c                                            # Heap management for malloc/newlib (auto-generated)
│
└── Startup/
    └── startup_stm32f411retx.s                              # Reset handler, vector table (includes TIM2_IRQHandler entry)
```

---

## ⚙️ How It Works

```
                 TIM2
                  │
             1 ms interrupt
                  │
                  ▼
          TIM2_IRQHandler()
                  │
       ┌──────────┼──────────┐
       ▼          ▼          ▼
    Green       Red       Button
    timer       timer       timer
       │          │          │
     500ms      1000ms       10ms
       │          │          │
       ▼          ▼          ▼
   green_ready red_ready button_ready
       │          │          │
       └──────────┼──────────┘
                  ▼
                main()
```

1. **GPIO Setup (`GPIO_Init`)** — Enables the GPIOC clock and configures PC0 as an input with an internal pull-up (button), and PC1, PC3, PC4 as outputs (three LEDs).
2. **Timer Setup (`TIM2_Init`)** — Enables the TIM2 clock, sets `PSC = 15` and `ARR = 999` to produce a 1 ms tick (see the timing calculation below), forces an immediate register load via `EGR`, clears any stale update flag, enables the update interrupt, enables TIM2's NVIC line, and starts the counter.
3. **Scheduler ISR (`TIM2_IRQHandler`)** — Fires every 1 ms. It clears the pending flag first, then increments three independent software counters (`green_timer`, `red_timer`, `button_timer`). Whenever a counter reaches its target period, the ISR sets that task's `_ready` flag and resets the counter to zero. The ISR does **not** perform any LED toggling or button reading itself — it only tracks time and raises flags.
4. **Task Loop (`main()`)** — Runs forever, checking each `_ready` flag on every pass. When a flag is set, `main()` executes that task's actual work (toggle an LED, or check-and-respond to the button) and then clears the flag back to 0, marking the task as "done" until its timer reaches the target again.
5. **Button Task with Edge Detection** — Every 10 ms, `button_ready` is set. `main()` reads the current PC0 state and compares it against `last_button_state` from the previous check. Only a transition from released (`1`) to pressed (`0`) — a falling edge — toggles PC1. Holding the button down does not repeatedly toggle the LED, because the state only transitions once per physical press.

---

## 🔍 Code Explanation

**`Src/main.c`**

| Function / Variable | Purpose |
|---|---|
| `GPIO_Init()` | Enables GPIOC clock; configures PC0 as input with pull-up (button); configures PC1, PC3, PC4 as outputs (LEDs). |
| `TIM2_Init()` | Enables TIM2 clock; sets `PSC`/`ARR` for a 1 ms tick; forces an immediate update via `EGR`; clears the update flag; enables the update interrupt; enables TIM2's NVIC line; starts the counter. |
| `TIM2_IRQHandler()` | The scheduler ISR. Clears `TIM2_SR` **first** (see the UIF-position note below), then increments all three software timers and sets the corresponding `_ready` flag whenever a timer reaches its target period. |
| `green_timer`, `red_timer`, `button_timer` | `static volatile uint32_t` software counters, incremented once per millisecond by the ISR, each tracking time toward its own task's period. |
| `green_ready`, `red_ready`, `button_ready` | `static volatile uint8_t` event/ready flags, set by the ISR when a task's period elapses and cleared by `main()` once that task has run. |
| `last_button_state` | `static uint8_t`, holds the button's state from the previous 10 ms check, used for falling-edge detection. |
| `main()` | Calls `GPIO_Init()` and `TIM2_Init()` once, then loops forever checking each `_ready` flag and running the corresponding task (Green LED toggle, Red LED toggle, or button polling with falling-edge detection) whenever it's set. |

**Register Table**

| Register | Bit | Purpose |
|---|---|---|
| `RCC_AHB1ENR` | 2 | GPIOC Clock Enable |
| `RCC_APB1ENR` | 0 | TIM2 Clock Enable |
| `GPIOC_MODER` | 0–1 | PC0 Mode = Input |
| `GPIOC_MODER` | 2–3 | PC1 Mode = General-Purpose Output |
| `GPIOC_MODER` | 6–7 | PC3 Mode = General-Purpose Output |
| `GPIOC_MODER` | 8–9 | PC4 Mode = General-Purpose Output |
| `GPIOC_PUPDR` | 0–1 | PC0 Pull Configuration = Pull-Up |
| `GPIOC_IDR` | 0 | Read PC0 (button state) |
| `GPIOC_ODR` | 1, 3, 4 | Drive PC1, PC3, PC4 (LEDs) |
| `TIM2_PSC` | 0–15 | Prescaler Value = 15 |
| `TIM2_ARR` | 0–15 | Auto-Reload Value = 999 |
| `TIM2_EGR` | 0 | Force Update Event (UG) |
| `TIM2_SR` | 0 | Update Interrupt Flag (UIF) |
| `TIM2_DIER` | 0 | Update Interrupt Enable (UIE) |
| `TIM2_CR1` | 0 | Counter Enable (CEN) |
| `NVIC_ISER0` | 28 | Enable IRQ28 (TIM2 global interrupt) |

**Timing calculation (final configuration):**
```
Timer Clock (APB1, default HSI) = 16 MHz
PSC = 15   →  16 MHz / (15 + 1) = 1 MHz   →  1 µs per counter tick
ARR = 999  →  1000 counter ticks × 1 µs   =  1 ms Update period
```
Compared to `04_TIM2_Interrupt_LED_Toggle`'s configuration (`PSC = 15999`, `ARR = 999`, giving a 1-second period directly), this project deliberately uses a **1 ms tick** as the shared system time base, with the actual task periods (500 ms, 1000 ms, 10 ms) implemented in software by counting ticks — this is what makes multiple independent task periods possible from a single hardware timer.

---

## 🧠 Cooperative Scheduler Concept

This is the central concept of the project:

```
             TIM2
              │
          System tick
              │
              ▼
        Update timers
              │
              ▼
       Set ready flags
              │
              ▼
            main()
              │
       ┌──────┼──────┐
       ▼      ▼      ▼
    Task A  Task B  Task C
```

Tasks aren't forcibly interrupted by a scheduler the way a preemptive RTOS would — instead, `main()` **cooperatively** checks "Is Green ready? Is Red ready? Is Button ready?" on every pass through its loop, and executes whichever tasks are currently ready. This is why it's called a *cooperative* scheduler: each task runs to completion and control returns to the loop, rather than being preempted mid-execution.

**Why not just do everything inside the ISR?** It would have been possible to toggle both LEDs and read the button directly inside `TIM2_IRQHandler()`. That's deliberately not what this project does — the point was to learn the pattern of keeping the ISR short (a small amount of time-critical bookkeeping, then setting flags and returning) while the actual application logic runs in `main()`. This scales much better as more tasks are added, since the ISR's execution time stays roughly constant regardless of how much work each task does.

**Why software timers matter:**
```
Hardware timer
      ↓
System tick

Software timer
      ↓
Task period
```
TIM2 doesn't know anything about "Green LED" or "Red LED" — it only provides the shared 1 ms time base. Each task's actual period is entirely a software concept, tracked by its own counter.

**Why the ready flags must be cleared:** A `_ready` flag follows a simple lifecycle, not a persistent state:
```
0 → waiting
1 → task requested
0 → task consumed request
```
If `main()` set a flag but never cleared it back to 0 after running the task, the task would appear "ready" forever and re-run on every single loop iteration instead of once per period.

**Why the button became a periodic task instead of an EXTI interrupt:** Unlike `03_Interrupt_LED_Toggle`, this project deliberately checks the button every 10 ms from within the scheduler rather than using a hardware interrupt on a GPIO edge. This distinguishes two separate questions: `button_ready` answers *"when should I check the button?"*, while `GPIOC_IDR` answers *"what is the button's state right now?"* — a pattern common in polling-based cooperative schedulers.

---

## 🐛 Problems Faced & Debugging Journey

**1. Button toggled rapidly instead of once per press**
The original button logic toggled PC1 directly whenever PC0 read LOW:
```c
if ((*GPIOC_IDR & (1 << 0)) == 0) {
    *GPIOC_ODR ^= (1 << 1);
}
```
Since the scheduler checks the button every 10 ms, holding it down for one second produced roughly 100 checks (`1000 ms / 10 ms`), and each one toggled the LED — meaning the code was detecting *"button is currently pressed"* rather than *"button has just been pressed."*

**Fix — falling-edge detection:** A `last_button_state` variable was introduced to remember the previous reading. Only a `1 → 0` transition (released → pressed) counts as a new press and triggers a toggle; a steady `0 → 0` reading means the button is still being held, and no further toggling occurs. This gives the correct behavior for a clean transition: a detected falling edge produces one LED toggle. Note that this is edge detection, not switch debouncing — a mechanically bouncy switch can itself produce multiple rapid `1 → 0 → 1 → 0` transitions within a single physical press, and this logic would treat each clean falling edge it sees as a separate press. True one-press-per-toggle behavior would require adding debounce timing on top of this edge detection (see "Future Improvements").

**2. `ARR = 0` didn't behave correctly on real hardware**
An initial experiment used `TIMER_PERIOD = 0` together with `PSC = 15999`. This didn't behave properly in the scheduler configuration on physical hardware. The project moved to a more conventional configuration instead — `PSC = 15`, `ARR = 999` — which, assuming a 16 MHz TIM2 clock, cleanly produces a 1 µs counter tick and a 1 ms update period. This configuration is also considerably easier to reason about than the `ARR = 0` edge case.

**3. UIF clear position mattered**
The line `*TIM2_SR &= ~(1 << 0);` was originally placed at the **end** of the ISR. After testing, moving it to the **beginning** produced correct behavior. The resulting ISR structure:
```c
void TIM2_IRQHandler(void)
{
    *TIM2_SR &= ~(1 << 0);   // Clear UIF first
    // Scheduler work follows
}
```
**Lesson:** always acknowledge/clear the interrupt source correctly, early in the ISR, rather than as an afterthought at the end.

**4. Hardware fault — a faulty LED**
At one point, it looked like the scheduler and button logic weren't working at all. Isolating the hardware revealed that an LED itself was faulty; replacing it resolved the issue. **Lesson:** don't assume every failure is a software problem — check the hardware layer too.

---

## 🔬 Systematic Debugging Methodology

When the interrupt-driven scheduler wasn't producing the expected result, the system was debugged one layer at a time rather than changing everything at once:

```
                 Problem
                    │
                    ▼
             Isolate hardware
                    │
                    ▼
                 GPIO
                    │
                    ▼
              TIM2 polling
                    │
                    ▼
             TIM2 interrupt
                    │
                    ▼
              Scheduler
                    │
                    ▼
             Button logic
```

- **TIM2 in isolation, via polling** — `main()` checked `UIF` directly (no interrupt) and toggled an LED manually. The LED blinked correctly, confirming TIM2 clock, `PSC`, `CNT`, `ARR`, and the Update Event were all working.
- **TIM2 interrupt path in isolation** — Using an interrupt counter, the full chain (`UIF → DIER → NVIC IRQ28 → TIM2_IRQHandler()`) was verified to work correctly on its own.
- **Button and LED in isolation, without TIM2** — PC0 (button, pull-up) and PC1 (LED) were tested directly against `GPIOC_IDR`/`GPIOC_ODR` with no timer involved at all, confirming the button wiring, pull-up, and LED output were all correct.

This confirmed the problem was isolated to the scheduler/button logic itself (the rapid-toggle issue above) rather than any lower layer — a systematic isolation process that is a strong embedded debugging habit.

---

## 📊 Project 04 vs. Project 05

| | Project 04 | Project 05 |
|---|---|---|
| Timer | TIM2 interrupt | TIM2 interrupt |
| Task count | One main task | Multiple tasks |
| ISR responsibility | Toggles LED directly | Schedules tasks (updates timers, sets flags) |
| Time base | Hardware timer, 1-second period | Hardware timer, 1 ms system tick |
| Interrupt infrastructure | UIF / NVIC | UIF / NVIC |
| ISR-to-main handoff | Direct action in ISR | Ready flags |
| Complexity | Simple interrupt demo | Cooperative scheduler |

```
Project 04:  TIM2 → ISR → LED
Project 05:  TIM2 → ISR → software timers → ready flags → main() → tasks
```

---

## 💡 Skills Gained

**TIM2**
- PSC controls counter speed; ARR controls the number of counter ticks before an update event
- CNT is the actual running timer counter
- UIF indicates an update event has occurred
- EGR/UG can force an update event immediately
- DIER enables update interrupts
- TIM2 uses IRQ28 in the NVIC
- The NVIC connects the peripheral's interrupt request to the CPU

**GPIO**
- `MODER` selects input/output mode per pin
- `PUPDR` configures internal pull-up/pull-down resistors
- `IDR` is used to read input pin states
- `ODR` is used to drive output pin states

**Embedded C**
- `volatile` is essential for variables shared between an ISR and `main()`
- `static` gives software timer/flag variables file-level scope without exposing them globally
- Software counters, driven by one hardware tick, can create independent task periods
- Ready flags are a clean pattern for ISR-to-main communication

**Scheduler Concepts**
- The distinction between a *hardware timer* (system tick) and a *software timer* (task period) is one of the most important ideas in this project
- How a cooperative (non-preemptive) scheduler differs from putting all logic directly in an ISR

---

## 💻 Installation

**1. Clone the repository**
```bash
git clone https://github.com/anshukumar146/github.git
cd github/NUCLEOF411RE/05_Cooperative_Task_Scheduler
```

**2. Install STM32CubeIDE**
Download and install from [st.com/en/development-tools/stm32cubeide.html](https://www.st.com/en/development-tools/stm32cubeide.html) if not already installed.

**3. Import the project**
- Open STM32CubeIDE
- Go to **File → Open Projects from File System...**
- Select the `05_Cooperative_Task_Scheduler` folder
- Click **Finish** to import

---

## ▶️ Running the Project

1. Wire the hardware: a pushbutton on **PC0** (internal pull-up handles idle-high state — no external resistor needed), and three LEDs (each with a current-limiting resistor) on **PC1**, **PC3**, and **PC4**.
2. Connect the NUCLEO-F411RE board to your computer via USB.
3. In STM32CubeIDE, select the project in the Project Explorer.
4. Click **Build** to compile.
5. Click **Run → Debug** (using the `05_Cooperative_Task_Scheduler Debug.launch` configuration) to flash the firmware via ST-Link.
6. Once flashed, the scheduler runs automatically.

---

## 🧑‍💻 Usage

- **Green LED (PC3)** toggles automatically every 500 ms, indefinitely.
- **Red LED (PC4)** toggles automatically every 1000 ms, indefinitely.
- **Press the button (PC0)** → the third LED (PC1) toggles once per detected falling edge, regardless of how long the button is held down.
- All three tasks are independently scheduled from the same 1 ms system tick and executed sequentially by `main()`. The scheduler itself introduces no blocking delay; however, a long-running task in `main()` can delay execution of other ready tasks, which is an inherent limitation of cooperative scheduling.

---


## 🖼️ Schematic diagram

<img width="1218" height="618" alt="Screenshot 2026-08-20 223048" src="https://github.com/user-attachments/assets/e0025af9-b398-4109-bee6-45ba5fe1cc40" />


Note: R4 provide external pull-ups for PC0. In the current firmware, the STM32's internal pull-ups are also enabled via GPIOC_PUPDR, so these external resistors are optional and can be removed. They are included here to illustrate an alternative hardware pull-up implementation and to ensure the circuit would still operate even if the internal pull-ups were not configured.

---


## 🖼️ Screenshots

## Output Screenshot
<img width="1260" height="720" alt="05_Cooperative_Task_Scheduler" src="https://github.com/user-attachments/assets/e1d6c7cc-5849-479c-af77-a4d6fe223ec9" />


---

## 🎥 Demo Video




https://github.com/user-attachments/assets/7d11b150-c176-4250-aa53-e679bb56d681




---

## 📊 Results

- The Green LED reliably toggles every 500 ms and the Red LED every 1000 ms, each independently scheduled from the same 1 ms system tick and executed sequentially by `main()`.
- The button produces one LED toggle per detected falling edge, thanks to edge detection — holding it down no longer causes rapid re-toggling. This does not yet guarantee one toggle per physical press if the switch itself bounces (see "Future Improvements").
- The ISR remains short because it performs only scheduler bookkeeping (updating timers and setting flags); the execution time of individual application tasks in `main()` does not directly increase ISR execution time.
- Each subsystem (GPIO, TIM2 polling, TIM2 interrupt, scheduler, button logic) was independently verified working through isolated testing.

---

## 🚀 Future Improvements

- Add more tasks to the scheduler (e.g., a UART heartbeat message, a third LED with its own period) to further demonstrate scalability
- Introduce simple task state machines for more complex per-task behavior beyond a single toggle
- Add proper switch-debounce logic to reject mechanical contact bounce, so a single physical press is guaranteed to produce a single detected falling edge
- Explore PWM generation using TIM2 or another timer, building on the PSC/ARR knowledge from this project
- Investigate RTOS concepts (e.g., FreeRTOS) as the natural next step beyond a hand-rolled cooperative scheduler, once task count and complexity grow further
- Document the earlier `ARR = 0` failure mode more precisely (e.g., what specifically went wrong on hardware) if revisited, since the notes describe the symptom but not the root cause in detail

---

## 🎓 Learning Outcomes

By studying and building this project, you will learn:

- How to use a single hardware timer as a shared system tick for multiple independent tasks
- The difference between a hardware timer (system tick) and software timers (individual task periods)
- The ready-flag pattern for clean ISR-to-main communication
- Why interrupt handlers should stay short, with actual application logic deferred to the main loop
- How to implement proper falling-edge detection to distinguish "just pressed" from "currently held down"
- Why clearing a peripheral's interrupt flag early in the ISR matters
- A systematic, layer-by-layer approach to debugging embedded systems (hardware → GPIO → timer polling → timer interrupt → scheduler → application logic)
- The conceptual foundation that cooperative schedulers provide before adopting a full RTOS

---

## 🧩 Skills Demonstrated

- Bare-Metal Embedded C Programming
- ARM Cortex-M Timer Peripheral Configuration (TIM2)
- Cooperative Task Scheduler Design
- Interrupt Handling (NVIC, ISR design, ISR-to-main communication)
- Button Polling and Falling-Edge Detection
- Systematic, Layered Hardware/Software Debugging
- STM32 Peripheral Clock & Register-Level Configuration
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
- The STM32F411xC/E Reference Manual for TIM2, RCC, GPIO, and NVIC register documentation
- The embedded systems community for resources on cooperative scheduling and bare-metal ARM Cortex-M timer programming

---

<div align="center">

⭐ If you found this repository helpful, consider giving it a star!

</div>
