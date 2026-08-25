# 🎚️ STM32 Bare-Metal Servo Position Controller

A bare-metal STM32 project that drives an SG90 servo using TIM2 PWM and a push button — configured entirely through direct register manipulation (GPIO, RCC, TIM2), with no HAL or Arduino-style servo library. The project also documents the hardware debugging process, using a multimeter and logic analyzer to verify PWM frequency, period, and pulse widths.

![STM32](https://img.shields.io/badge/MCU-STM32F411RE-03234B?style=for-the-badge&logo=stmicroelectronics&logoColor=white)
![Bare Metal](https://img.shields.io/badge/Firmware-Bare--Metal%20C-blue?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Active-brightgreen?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)

---

## 📋 Specifications

| Property | Value |
|---|---|
| **Board** | STM32 Nucleo-F411RE |
| **Programming Model** | Bare-metal C (direct register access, no HAL) |
| **Timer / Channel** | TIM2, Channel 2 |
| **PWM Pin** | PA1 (AF1 → TIM2_CH2) |
| **PWM Frequency** | 50 Hz (20 ms period) |
| **Input** | Push-button on PA4 (internal pull-up, software-debounced) |
| **Actuator** | SG90 servo motor |
| **Debug Tools** | Digital multimeter, logic analyzer (PulseView) |

---

## 📖 Project Overview

**Purpose**
This project controls an SG90 servo using an STM32 microcontroller by generating a standard 50 Hz servo PWM signal at the register level — no `HAL_TIM_PWM_*` calls and no Arduino `Servo.h`. A push button cycles the servo through three fixed positions (~0°, ~90°, ~180°), and every step of the design — from timer math to signal verification — is worked out and documented explicitly.

**Real-World Applications**
- A reference for learning register-level STM32 timer/PWM configuration
- A base for any bare-metal servo, motor, or PWM-driven actuator project
- A worked example of systematic hardware debugging (multimeter → logic analyzer → mechanical inspection)

**Target Users**
- Embedded systems students learning STM32 register-level programming
- Developers who want to understand PWM generation without abstraction layers
- Anyone debugging a "servo isn't moving" problem who wants a methodical troubleshooting reference

---

## 🎯 Project Objectives

- Learn STM32 GPIO register configuration
- Learn STM32 timer configuration at the register level
- Generate a 50 Hz PWM signal
- Control an SG90 servo without a servo library
- Understand timer prescaler and auto-reload registers
- Understand the relationship between PWM pulse width and servo position
- Implement a push-button interface with software debounce and edge detection
- Debug hardware using a multimeter and verify PWM using a logic analyzer
- Diagnose a mechanical problem instead of incorrectly modifying firmware

---

## 🛠️ Hardware Used

| Component | Purpose |
|---|---|
| STM32 Nucleo / STM32F4 board | Main microcontroller |
| SG90 servo | Controlled actuator |
| Push button | Position selection |
| Logic analyzer | PWM verification |
| Digital multimeter | Voltage/power verification |
| Breadboard | Prototyping |
| Jumper wires | Connections |
| 5V supply | Servo power |

---

## 🔌 Connections

**Servo**

| Servo Wire | Connection |
|---|---|
| Red | +5V |
| Brown/Black | GND |
| Yellow/Orange | STM32 PWM output (PA1 → TIM2_CH2) |

**Push Button**

```
PA4 → Push Button
```

The button input uses an internal pull-up:
- Button released → PA4 = HIGH
- Button pressed → PA4 = LOW

**Logic Analyzer**

```
STM32 PA1  ───── Logic Analyzer input
STM32 GND  ───── Logic Analyzer GND
```

---

## ⚙️ PWM Configuration

PA1 is configured for **Alternate Function 1 (TIM2_CH2)**.

| Register | Purpose |
|---|---|
| `TIM2_PSC` | Prescaler |
| `TIM2_ARR` | PWM period |
| `TIM2_CCR2` | PWM pulse width |
| `TIM2_CCMR1` | PWM mode |
| `TIM2_CCER` | Enables output |
| `TIM2_CR1` | Starts timer |
| `TIM2_EGR` | Generates update event |

**Timer calculation** (assuming a 16 MHz timer clock):

```
PSC = 15   →   timer freq = 16 MHz / (15 + 1) = 1 MHz   →   1 timer count = 1 µs
ARR = 19999   →   period = (19999 + 1) × 1 µs = 20 ms   →   frequency = 50 Hz
```

**Servo position → CCR2**

| Servo Position | CCR2 | HIGH Time | LOW Time | Period | Frequency |
|---|---|---|---|---|---|
| ~0° | 1000 | 1 ms | 19 ms | 20 ms | 50 Hz |
| ~90° | 1500 | 1.5 ms | 18.5 ms | 20 ms | 50 Hz |
| ~180° | 2000 | 2 ms | 18 ms | 20 ms | 50 Hz |

---

## 🔘 Button Control & State Machine

The servo cycles through three fixed positions on each button press:

```
Initial (~0°) → press → ~90° → press → ~180° → press → ~0° → repeat
```

A state variable `pos` maps to the three `CCR2` values (1000 / 1500 / 2000), advancing to the next state — and wrapping back to the first — on every detected press.

**Debounce & edge detection**

Mechanical switches don't produce a clean HIGH → LOW transition, so a single physical press can look like several to the MCU. The firmware:
1. Reads the button.
2. Checks whether it's LOW.
3. Waits briefly.
4. Reads it again.
5. Accepts the press only if it's still LOW.

A new press is only registered on a `last_button_state == 1 && current_button_state == 0` transition (HIGH → LOW), which stops the servo from cycling continuously while the button is held down.

---

## 🧩 Register-Level Configuration

- **RCC** — `AHB1ENR` enables the GPIOA clock; `APB1ENR` enables the TIM2 clock, both before GPIOA/TIM2 are touched.
- **GPIOA** — PA1 is set to alternate-function mode (AF1 → TIM2_CH2); PA4 is set as a digital input with an internal pull-up.
- **TIM2** — Channel 2 is configured in **PWM Mode 1**, with `PSC = 15`, `ARR = 19999`, and `CCR2` set to 1000/1500/2000 depending on the selected state. Output is routed to PA1 via TIM2_CH2.

---

## 🧪 Testing Procedure

Testing was done progressively rather than wiring everything at once and assuming it worked:

1. **Arduino servo test** — The SG90 was first driven with Arduino's `Servo` library at 0°/90°/180° to confirm the servo itself worked.
2. **Servo power check** — The servo supply was measured with a multimeter (~5.04V, and ~5.07V in an earlier test), confirming it was receiving the expected 5V.
3. **STM32 PWM test** — The servo was then driven directly from STM32 via TIM2, testing `CCR2 = 1000`, `1500`, and `2000` individually.

---

## 🔍 Debugging Journey

Initial symptoms were confusing: Arduino control worked and the STM32 PWM appeared correctly configured, but the servo sometimes buzzed, sometimes stayed in one position, and became warm during some tests — suggesting (incorrectly) that the PWM configuration might be wrong.

A multimeter confirmed the servo supply (~5V) and the STM32 GPIO logic level (~3.3V) were both as expected, which ruled out a basic power problem. The PWM itself was then checked with a Saleae-compatible 8-channel logic analyzer using PulseView, which confirmed:

| Test Point | Measured HIGH | Measured LOW | Period | Frequency |
|---|---|---|---|---|
| 1.0 ms PWM | ~1 ms | ~19 ms | ~20 ms | ~50 Hz |
| 1.5 ms PWM | ~1.5 ms | ~18.5 ms | ~20 ms | ~50 Hz |
| 2.0 ms PWM | ~2 ms | ~18 ms | ~20 ms | ~50 Hz |

This proved the STM32 was generating the correct waveform — so the problem wasn't firmware.

**The actual problem** turned out to be mechanical: the servo horn was too tight and mechanically restricted the servo's movement, which produced buzzing, apparent stalling, and warming — even though the PWM signal was perfectly correct. Adjusting/rotating the horn resolved the issue.

> **Lesson:** A correct PWM waveform does not automatically guarantee mechanical movement. When debugging embedded hardware, work through the stack systematically: software → timer configuration → GPIO output → signal waveform → power supply → connections → mechanical system.

**⚠️ Servo heating note:** A servo that's mechanically blocked can continuously apply torque against the obstruction and heat up. Don't leave a buzzing or stalled servo powered for long — disconnect it, let it cool, and fix the obstruction before re-testing.

---

## ❌ Problems Encountered and Fixes

| Problem | Investigation | Fix |
|---|---|---|
| Servo initially didn't move | Checked Arduino first | Arduino confirmed servo worked |
| STM32 servo didn't initially move | Checked supply and PWM | Logic analyzer used |
| Servo produced buzzing sound | Checked PWM and power | Investigated mechanical movement |
| Servo became warm | Suspected stall | Stopped prolonged testing |
| PWM uncertainty (multimeter can't show timing) | Needed pulse-width verification | Used logic analyzer |
| Servo horn restricting movement | Mechanical inspection | Horn adjusted |
| Button could produce multiple transitions | Switch bounce | Software debounce |
| Button held down could repeatedly change state | Needed edge detection | HIGH → LOW press detection |

---

## 🧪 Final Test Results

| Test | Result |
|---|---|
| Arduino servo test | ✅ Passed |
| 5V servo supply | ✅ Passed |
| STM32 PWM generation | ✅ Passed |
| 1 ms / 1.5 ms / 2 ms PWM | ✅ Passed |
| ~50 Hz PWM | ✅ Passed |
| Logic analyzer verification | ✅ Passed |
| Servo movement | ✅ Passed |
| Button input & debounce | ✅ Passed / Implemented |
| Position cycling | ✅ Working |
| Mechanical horn issue | ✅ Fixed |

---

## 📁 Repository Structure

```
STM32_Servo_Controller/
├── README.md
├── Core/
│   └── main.c
├── Documentation/
│   ├── pwm_1ms.png
│   ├── pwm_1_5ms.png
│   ├── pwm_2ms.png
│   └── pwm_comparison.png
├── Hardware/
│   ├── wiring.jpg
│   └── setup.jpg
└── Logic_Analyzer/
    └── pulseview_captures/
```

---

## 🏁 Final Result

```
             STM32
               │
        ┌──────┴──────┐
        │             │
       PA1           PA4
        │             │
   TIM2_CH2         Button
        │
        │ PWM
        ▼
      SG90
      Servo
```

The completed project generates a stable 50 Hz / 20 ms PWM signal with selectable pulse widths (1.0 ms → ~0°, 1.5 ms → ~90°, 2.0 ms → ~180°), independently verified with a logic analyzer, and successfully drives the servo after resolving a mechanical horn restriction.

---

## 🖼️ Screenshots

## Servo PWM Logic Analyzer Comparison

<img width="1897" height="774" alt="STM32_Servo_PWM_Logic_Analyzer_Comparison" src="https://github.com/user-attachments/assets/b4d9cc9f-68e5-4fff-b7d3-4836efed4081" />


---
## Output Screenshot

<img width="700" height="500" alt="Screenshot 2026-08-24 225421" src="https://github.com/user-attachments/assets/a9fa11f8-4c17-4224-be66-345ebc40c1bc" />


---

## 🎥 Demo Video





https://github.com/user-attachments/assets/1258b21d-3c28-4389-859e-63a55c3ba2bd





---
## 📚 What I Learned

**STM32**
Memory-mapped registers, RCC clock enabling, GPIO mode/pull-up configuration, Alternate Function configuration, timer configuration, PWM generation, prescaler and auto-reload registers, capture/compare registers, PWM Mode 1.

**Embedded C**
`volatile`, pointer-based register access, bit manipulation, masks, bit shifting, state machines, functions, software debounce, edge detection.

**Electronics**
Servo power requirements, common ground, 3.3V logic vs. 5V actuator supply, PWM frequency/duty cycle/pulse width, electrical vs. mechanical debugging.

**Debugging**
The biggest lesson: don't assume the firmware is wrong just because the hardware isn't moving — measure the signal and systematically isolate the problem. The logic analyzer proved the PWM was correct, and mechanical inspection eventually revealed the real cause.

---

## 🚀 Future Improvements

- Replace software delay debounce with a timer-based debounce
- Use a proper millisecond time base instead of a busy-wait delay
- Add more servo positions
- Use a potentiometer to control servo angle
- Implement ADC → servo position control
- Add UART commands for servo control
- Use a timer interrupt instead of continuously polling the button
- Add limit protection for servo movement
- Create a reusable bare-metal PWM driver
- Control multiple servos using multiple timer channels
- Add a proper external 5V servo power supply
- Add a PCB version of the project

---

## 🧩 Skills Demonstrated

- Bare-Metal STM32 Register Programming (RCC, GPIO, TIM2)
- PWM Signal Generation & Timer Math
- Embedded C (pointers, bitwise operations, state machines)
- Hardware Debugging (multimeter + logic analyzer workflow)
- Systematic Root-Cause Analysis (electrical vs. mechanical)
- Technical Documentation

---

## 👤 Author

**Name:** Anshu Kumar
**GitHub:** [@anshukumar146](https://github.com/anshukumar146)
**LinkedIn:** _(add your LinkedIn URL here)_
**Email:** _(add your email here)_

---

## 📜 License

This project is licensed under the **MIT License**.

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

<div align="center">

⭐ If you found this repository helpful, consider giving it a star!

</div>
