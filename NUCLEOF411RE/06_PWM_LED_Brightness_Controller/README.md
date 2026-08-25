# 💡 STM32 NUCLEO-F411RE — Bare-Metal 5-Level PWM LED Brightness Controller

**Level:** Bare-Metal Advanced

A register-level STM32F411RE project that generates a 1 kHz PWM signal on TIM2 Channel 1 to control LED brightness across five selectable levels, chosen via five debounced push buttons — with no HAL, LL libraries, or CubeMX-generated code.

![STM32](https://img.shields.io/badge/MCU-STM32F411RE-03234B?style=for-the-badge&logo=stmicroelectronics&logoColor=white)
![Language](https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c&logoColor=white)
![IDE](https://img.shields.io/badge/IDE-STM32CubeIDE-blue?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Active-brightgreen?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)

---

## 📖 Project Overview

This project implements a PWM-based LED brightness controller using an STM32 microcontroller, programmed entirely at the register level. Five push buttons select different PWM duty cycles, and the selected duty cycle controls the brightness of an LED connected to PA0, configured as TIM2 Channel 1. The project also implements software switch debouncing to prevent mechanical switch bounce from causing unwanted input detection.

**Objective**
Understand how an STM32 microcontroller can generate PWM and control an LED without relying on high-level libraries. This project specifically demonstrates:
- Enabling peripheral clocks through RCC
- Configuring GPIO pins using `MODER` and `PUPDR`
- Configuring Alternate Function mode
- Connecting a GPIO pin to a timer channel
- Configuring TIM2 for PWM generation
- Understanding `PSC`, `ARR`, and `CCR` registers
- Reading push buttons through GPIO `IDR`
- Implementing software debounce
- Dynamically changing PWM duty cycle

**Real-World Applications**
PWM-based brightness/intensity control is used throughout real embedded systems — LED dimming, motor speed control, fan speed regulation, servo positioning, and any application needing variable analog-like output from a purely digital pin.

**Target Users**
- Embedded systems students learning STM32 timer PWM generation at the register level
- Developers wanting a from-scratch example of GPIO Alternate Function → Timer Channel wiring
- Anyone using a NUCLEO-F411RE board wanting a practical multi-input, PWM-output project without HAL

---

## ✨ Features

✅ Register-level STM32 programming — no HAL, no LL, no CubeMX-generated peripheral code
✅ TIM2 PWM generation at a 1 kHz frequency
✅ Five push buttons, each selecting a distinct PWM duty cycle level
✅ Five selectable brightness levels: 0%, 25%, 50%, 75%, and ~100%
✅ Internal GPIO pull-up resistors on all five switch inputs — no external resistors required
✅ Software switch debouncing on every button read
✅ Defined switch priority handling for simultaneous button presses
✅ GPIO Alternate Function configuration connecting PA0 to TIM2 Channel 1

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

| Component | Quantity |
|---|---|
| STM32 NUCLEO-F411RE development board | 1 |
| LED | 1 |
| Push buttons | 5 |
| Resistors / current-limiting components | As required |
| Breadboard | 1 |
| Jumper wires | As required |
| USB cable | 1 |

### LED (PWM Output)

| Function | STM32 Pin | Peripheral |
|---|---|---|
| PWM LED output | PA0 | TIM2_CH1 |

PA0 is configured in Alternate Function mode with AF1 selected, connecting it to TIM2 Channel 1.

### Switches

| Switch | Pin | GPIO Port | Active State |
|---|---|---|---|
| SW1 | PC3 | GPIOC | LOW |
| SW2 | PC0 | GPIOC | LOW |
| SW3 | PC1 | GPIOC | LOW |
| SW4 | PA1 | GPIOA | LOW |
| SW5 | PA4 | GPIOA | LOW |

All switches use internal pull-up resistors, so:
```
Switch released → GPIO reads 1
Switch pressed  → GPIO reads 0
```

### Switch-to-PWM Mapping

| Switch | Pin | CCR1 | Duty Cycle |
|---|---|---|---|
| SW1 | PC3 | 500 | 50% |
| SW2 | PC0 | 999 | ~100% |
| SW3 | PC1 | 750 | 75% |
| SW4 | PA1 | 250 | 25% |
| SW5 | PA4 | 0 | 0% |
| No switch pressed | — | 0 | 0% |

---

## 📂 Folder Structure

```
06_PWM_LED_Brightness_Controller/
│
├── .project                                                 # Eclipse/STM32CubeIDE project descriptor
├── .cproject                                                 # Build configuration (Debug/Release, compiler/linker options)
├── 06_PWM_LED_Brightness_Controller Debug.launch            # Debug launch configuration (ST-Link settings)
├── STM32F411RETX_FLASH.ld                                   # Linker script — runs code from Flash
├── STM32F411RETX_RAM.ld                                     # Linker script — runs code from RAM (debug-in-RAM)
│
├── Src/
│   ├── main.c                                                 # GPIO, TIM2 PWM configuration, switch reading, and debounce
│   ├── syscalls.c                                             # Newlib syscall stubs (auto-generated)
│   └── sysmem.c                                               # Heap management for malloc/newlib (auto-generated)
│
└── Startup/
    └── startup_stm32f411retx.s                                 # Reset handler, vector table
```

---

## ⚡ PWM Principle

PWM stands for Pulse Width Modulation. Instead of continuously varying the voltage supplied to the LED, the microcontroller rapidly switches the signal between HIGH and LOW. The percentage of time the signal remains HIGH determines the average power delivered to the LED — this percentage is the **duty cycle**:

```
Duty Cycle = (T_HIGH / T_PWM) × 100
```

For example, a 50% duty cycle means the signal is HIGH for 50% of each period and LOW for the other 50%.

---

## ⚙️ How It Works

```
SW1–SW5 → GPIO IDR → Debounce → Switch_Read() → TIM2_CCR1 → TIM2 PWM → PA0 → LED
```

1. **GPIO Setup (`GPIO_Init`)** — Enables the GPIOA and GPIOC clocks, configures PC0/PC1/PC3 and PA1/PA4 as inputs, and enables internal pull-up resistors on all five switch pins.
2. **Timer + PWM Setup (`TIM2_Init`)** — Enables the TIM2 clock, configures PA0 as Alternate Function (AF1, connecting it to TIM2_CH1), sets `PSC`/`ARR` for a 1 kHz PWM frequency, configures Channel 1 for PWM Mode 1 with preload enabled, enables the channel output, forces an update event, and starts the counter.
3. **Main Loop** — `main()` calls `GPIO_Init()` and `TIM2_Init()` once, then loops forever calling `Switch_Read()`.
4. **Switch Reading and Priority (`Switch_Read`)** — Checks each of the five switches in a fixed priority order (SW1 → SW2 → SW3 → SW4 → SW5) via `Switch_Debounce()`, and sets `TIM2_CCR1` to the duty-cycle value corresponding to the first pressed switch found. If none are pressed, `CCR1` is set to 0 (LED off).
5. **Debouncing (`Switch_Debounce`)** — Reads a switch pin; if it reads pressed (active-low), waits via a software delay, then re-reads the same pin. Only if the switch is still pressed after the delay does the function report a confirmed press.
6. **PWM Output** — TIM2 continuously compares its running counter (`CNT`) against `CCR1`. Whenever `CNT < CCR1`, the output is HIGH; otherwise it's LOW — this comparison, repeated every PWM period, produces the duty-cycle-controlled square wave on PA0 that drives the LED's brightness.

---

## 🔍 Timer Configuration & Calculations

**Prescaler (PSC):**
```
F_CNT = F_TIM / (PSC + 1)
```
With `PSC = 15` and an assumed TIM2 input clock of 16 MHz:
```
F_CNT = 16 MHz / (15 + 1) = 1 MHz   →  each timer count ≈ 1 µs
```

**Auto-Reload Register (ARR):**
The timer counts from `0` to `ARR`, giving `ARR + 1` counts per PWM period. With `ARR = 999`:
```
Counts per period = 999 + 1 = 1000
F_PWM = 1 MHz / 1000 = 1 kHz
T_PWM = 1 / 1 kHz = 1 ms
```

**PWM Duty Cycle (via CCR1):**
Because `ARR = 999`, the period contains 1000 counts, so `CCR1` maps directly to approximate percentage:

| CCR1 | Approx. Duty Cycle |
|---|---|
| 999 | ~100% |
| 750 | 75% |
| 500 | 50% |
| 250 | 25% |
| 0 | 0% |

**PWM Mode 1 behavior** (selected via `OC1M = 110`):
```
CNT < CCR1 → Output HIGH
CNT ≥ CCR1 → Output LOW
```
Example: `CCR1 = 500`, `ARR = 999` → HIGH for 500 of every 1000 counts → approximately 50% duty cycle.

**PWM Preload (`OC1PE = 1`):** Enables the `CCR1` preload register, so a new compare value is transferred to the active register at the next update event rather than causing an uncontrolled immediate change mid-period. This matters specifically because this project changes duty cycle dynamically at runtime as different switches are pressed.

**PA0 Alternate Function:**
```
MODER0 = 10   → Alternate Function mode
AFR0   = 0001 → AF1, which connects PA0 to TIM2_CH1
```

---

## 🔍 Code Explanation

**`Src/main.c`**

| Function | Purpose |
|---|---|
| `GPIO_Init()` | Enables GPIOA/GPIOC clocks; configures PC0, PC1, PC3, PA1, PA4 as inputs with internal pull-ups (the five switches). |
| `TIM2_Init()` | Enables the TIM2 clock; configures PA0 as Alternate Function AF1 (TIM2_CH1); sets `PSC`/`ARR` for a 1 kHz PWM period; configures Channel 1 for PWM Mode 1 with preload enabled (`CCMR1`); enables the channel output (`CCER`); forces an update event (`EGR`); starts the counter (`CR1`). |
| `Switch_Read()` | Checks all five switches via `Switch_Debounce()` in a fixed `if / else-if` priority order and sets `TIM2_CCR1` to the corresponding duty-cycle value; defaults to 0 if no switch is pressed. |
| `Switch_Debounce(IDR, pin)` | A reusable debounce function taking an IDR register pointer and a pin number. Detects an active-low press, waits via `Delay()`, then re-checks the same pin before confirming the press. |
| `Delay(count)` | A simple busy-wait `for` loop used as the debounce delay. |
| `main()` | Calls `GPIO_Init()` and `TIM2_Init()` once, then loops forever calling `Switch_Read()`. |

**Register Table**

| Register | Bit(s) | Purpose |
|---|---|---|
| `RCC_AHB1ENR` | 0 | GPIOA Clock Enable |
| `RCC_AHB1ENR` | 2 | GPIOC Clock Enable |
| `RCC_APB1ENR` | 0 | TIM2 Clock Enable |
| `GPIOA_MODER` | 2–3, 8–9 | PA1, PA4 Mode = Input |
| `GPIOA_MODER` | 0–1 | PA0 Mode = Alternate Function |
| `GPIOC_MODER` | 0–1, 2–3, 6–7 | PC0, PC1, PC3 Mode = Input |
| `GPIOA_PUPDR` | 2–3, 8–9 | PA1, PA4 Pull Configuration = Pull-Up |
| `GPIOC_PUPDR` | 0–1, 2–3, 6–7 | PC0, PC1, PC3 Pull Configuration = Pull-Up |
| `GPIOA_AFRL` | 0–3 | PA0 Alternate Function = AF1 (TIM2_CH1) |
| `TIM2_PSC` | 0–15 | Prescaler Value = 15 |
| `TIM2_ARR` | 0–15 | Auto-Reload Value = 999 |
| `TIM2_CCMR1` | 3 | OC1PE — Output Compare 1 Preload Enable |
| `TIM2_CCMR1` | 4–6 | OC1M — Output Compare 1 Mode = 110 (PWM Mode 1) |
| `TIM2_CCER` | 0 | CC1E — Channel 1 Output Enable |
| `TIM2_CCR1` | 0–15 | Channel 1 Compare Value (Duty Cycle) |
| `TIM2_EGR` | 0 | UG — Force Update Event |
| `TIM2_CR1` | 0 | CEN — Counter Enable |

---

## 🧠 Switch Priority

`Switch_Read()` uses an `if / else-if / else-if / ...` structure, so if multiple switches are pressed simultaneously, the switch appearing first in the code takes priority:

```
SW1 → SW2 → SW3 → SW4 → SW5
```

For example, if SW1 and SW3 are pressed at the same time, SW1 wins because it's checked first. This behavior is intentional and documented here as the project's switch priority policy, rather than an unhandled edge case.

---

## 🐛 Software Debouncing

Mechanical switches don't produce a perfectly clean transition. When pressed, the contacts can rapidly bounce between HIGH and LOW:

```
Ideal:
HIGH ─────────┐
              │
              └──────── LOW

Actual:
HIGH ─────┐ ┌─┐ ┌────────
          └─┘ └─┘
              ↓
           Bounce
```

If the MCU reads the switch during this bounce window, it may misinterpret one physical press as multiple transitions.

**Debounce algorithm used:**
```
Read switch
     ↓
Pressed?
     ↓ YES
Delay
     ↓
Read again
     ↓
Still pressed?
     ↓ YES
Accept press
```

`Switch_Debounce()` is reusable across all five switches — it accepts an IDR register pointer and a pin number, e.g. `Switch_Debounce(GPIOC_IDR, 3)` checks PC3, while `Switch_Debounce(GPIOA_IDR, 4)` checks PA4.

**Software delay limitation:** The delay uses a simple busy-wait loop with a count of `10000`. This provides a rough debounce delay, but its actual duration is **not precisely calibrated to a specific millisecond value** — it depends on CPU clock frequency, compiler optimization level, and generated instruction overhead. For this project, it was sufficient for successful hardware testing, but a hardware-timer-based non-blocking debounce would be a more robust future approach (see "Future Improvements").

---

## 🏗️ Software Architecture

```
main()
 │
 ├── GPIO_Init()
 │
 ├── TIM2_Init()
 │
 └── while(1)
       │
       └── Switch_Read()
              │
              └── Switch_Debounce()
```

| Function | Responsible For |
|---|---|
| `GPIO_Init()` | GPIOA/GPIOC clock enable, GPIO input configuration, pull-up configuration |
| `TIM2_Init()` | TIM2 clock enable, PA0 alternate function, prescaler, ARR, PWM mode, PWM preload, channel enable, timer start |
| `Switch_Debounce()` | Checking switch state, waiting for bouncing to settle, confirming the press |
| `Switch_Read()` | Determining which switch is pressed, selecting the corresponding PWM duty cycle |

---

## 💻 Installation

**1. Clone the repository**
```bash
git clone https://github.com/anshukumar146/github.git
cd github/NUCLEOF411RE/06_PWM_LED_Brightness_Controller
```

**2. Install STM32CubeIDE**
Download and install from [st.com/en/development-tools/stm32cubeide.html](https://www.st.com/en/development-tools/stm32cubeide.html) if not already installed.

**3. Import the project**
- Open STM32CubeIDE
- Go to **File → Open Projects from File System...**
- Select the `06_PWM_LED_Brightness_Controller` folder
- Click **Finish** to import

---

## ▶️ Running the Project

1. Wire the hardware: five pushbuttons on PC0, PC1, PC3, PA1, and PA4 (internal pull-ups handle idle-high state — no external resistors needed for the switches), and an LED (with a current-limiting resistor) on **PA0**.
2. Connect the NUCLEO-F411RE board to your computer via USB.
3. In STM32CubeIDE, select the project in the Project Explorer.
4. Click **Build** to compile.
5. Click **Run → Debug** (using the `06_PWM_LED_Brightness_Controller Debug.launch` configuration) to flash the firmware via ST-Link.
6. Once flashed, the program runs automatically — press any switch to set the LED's brightness.

---

## 🧑‍💻 Usage

- **No switch pressed** → LED off (0% duty cycle)
- **SW1 (PC3)** → LED at 50% brightness
- **SW2 (PC0)** → LED at ~100% brightness
- **SW3 (PC1)** → LED at 75% brightness
- **SW4 (PA1)** → LED at 25% brightness
- **SW5 (PA4)** → LED at 0% brightness
- If multiple switches are pressed simultaneously, the switch checked first in the priority order (SW1 → SW2 → SW3 → SW4 → SW5) determines the LED's brightness.

---

## 🖼️ Screenshots

## Output Screenshot

<img width="322" height="400" alt="06_PWM_LED_Brightness_Controller" src="https://github.com/user-attachments/assets/b9a35718-16b2-4c5f-a95e-917bb8f25dd9" />

<img width="300" height="400" alt="IMG_20260815_232617" src="https://github.com/user-attachments/assets/eb3311f4-1a50-4fa2-a205-57420bf8f065" />

---

## 🎥 Demo Video



https://github.com/user-attachments/assets/b411c0d1-98c6-49c2-8126-696d2b3a1574



---

## 🧪 Testing Performed

**Individual switch testing**
| Switch | Expected | Result |
|---|---|---|
| SW1 | 50% PWM | ✅ |
| SW2 | ~100% PWM | ✅ |
| SW3 | 75% PWM | ✅ |
| SW4 | 25% PWM | ✅ |
| SW5 | 0% PWM | ✅ |

**Switch release** — No switch pressed → LED off ✅

**Simultaneous switch testing** — Multiple switches were pressed simultaneously to verify the priority logic; the first switch in the `if/else-if` chain correctly took priority.

**Debounce testing** — Software debounce was added and tested successfully on hardware.

---

## 📊 Results

The project successfully demonstrates a five-level LED brightness controller using STM32 TIM2 PWM. The microcontroller reads five active-low push buttons and dynamically changes the TIM2 Channel 1 compare value; the resulting PWM signal is output through PA0 and controls the LED's brightness.

Achieved:
- 1 kHz PWM frequency
- 0%, 25%, 50%, 75%, and ~100% duty-cycle levels
- Register-level peripheral configuration throughout
- Five working switch inputs with internal pull-up configuration
- Software debounce, tested successfully on hardware
- Deterministic switch priority handling for simultaneous presses

---

## ⚠️ Limitations

- The software delay used for debouncing is not precisely calibrated to a specific time value — its actual duration depends on CPU clock, compiler optimization, and instruction overhead
- Debouncing is blocking — `Switch_Debounce()`'s delay stalls `Switch_Read()` (and therefore the rest of the polling loop) while waiting
- Multiple simultaneous switches are handled via fixed priority rather than explicit conflict detection or rejection
- PWM duty-cycle values (0/250/500/750/999) are manually assigned rather than calculated from a percentage
- The timing calculations assume the expected TIM2 input clock configuration (16 MHz, consistent with the default HSI source and no custom `SystemClock_Config()`)

---

## 🚀 Future Improvements

- **Hardware-timer-based debounce** — use a timer instead of a CPU busy-loop for debounce timing
- **Non-blocking debounce** — implement a state machine with periodic sampling instead of a blocking delay
- **Duty-cycle abstraction** — calculate the `CCR1` compare value from a duty-cycle percentage instead of hardcoding values like 250, 500, 750, 999
- **Better simultaneous-button handling** — detect multiple pressed buttons explicitly and either reject the combination or make the existing priority policy configurable
- **PWM preload optimization** — review preload/update timing carefully for glitch-free behavior when changing duty cycle dynamically during active PWM output
- **Expand to a potentiometer** — use an ADC to provide continuously variable brightness instead of five fixed levels

---

## 🎓 Key Concepts Learned

- Memory-mapped peripheral registers and pointer-based register access
- `volatile` and its role in accessing hardware registers correctly
- GPIO configuration: mode, pull-up/pull-down, and Alternate Function
- Input Data Registers (`IDR`) and active-low input handling
- Timer peripherals: prescaler, auto-reload register, and compare registers
- PWM Mode 1 behavior and how `CNT` vs. `CCR1` comparison produces a duty cycle
- Timer update events and why `EGR`/`UG` matters at initialization
- Timer channel enable (`CCER`) and why the PWM signal won't reach the pin without it
- Software debouncing, including its blocking-delay tradeoffs
- Priority-based input handling for simultaneous events

---

## 🧩 Skills Demonstrated

- Bare-Metal Embedded C Programming
- ARM Cortex-M Timer PWM Configuration (TIM2, PWM Mode 1)
- GPIO Alternate Function Configuration
- Multi-Input Switch Handling with Priority Logic
- Software Switch Debouncing
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
- The STM32F411xC/E Reference Manual for TIM2, GPIO Alternate Function, and RCC register documentation
- The embedded systems community for resources on bare-metal ARM Cortex-M PWM generation

---

<div align="center">

⭐ If you found this repository helpful, consider giving it a star!

</div>
