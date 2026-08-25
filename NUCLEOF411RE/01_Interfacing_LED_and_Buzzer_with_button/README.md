# 🔧 STM32 NUCLEO-F411RE — Bare-Metal LED & Button Interfacing

A register-level (bare-metal) embedded C project for the STM32F411RE microcontroller that reads a pushbutton and toggles an LED — written directly against memory-mapped GPIO and RCC registers, with no HAL or CubeMX-generated driver layer.

![STM32](https://img.shields.io/badge/MCU-STM32F411RE-03234B?style=for-the-badge&logo=stmicroelectronics&logoColor=white)
![Language](https://img.shields.io/badge/Language-C-00599C?style=for-the-badge&logo=c&logoColor=white)
![IDE](https://img.shields.io/badge/IDE-STM32CubeIDE-blue?style=for-the-badge)
![Status](https://img.shields.io/badge/Status-Active-brightgreen?style=for-the-badge)
![License](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)

---

## 📖 Project Overview

**Purpose**
This project demonstrates bare-metal GPIO control on the STM32F411RE Nucleo board — configuring pins and reading/writing them by manipulating hardware registers directly, without relying on ST's HAL (Hardware Abstraction Layer) or LL (Low-Layer) drivers.

**Problem Solved**
Most beginner STM32 tutorials rely on HAL functions like `HAL_GPIO_WritePin()`, which hide what's actually happening at the register level. This project strips that abstraction away, showing exactly how clock enables, mode configuration, and pin toggling work at the memory-mapped register level — critical knowledge for embedded engineers who need to debug, optimize, or write drivers from scratch.

**Real-World Applications**
Direct register manipulation is used in performance-critical embedded firmware, bootloaders, low-level driver development, and any scenario where HAL overhead is unacceptable (tight timing loops, minimal memory footprint, or hardware not yet supported by HAL).

**Target Users**
- Embedded systems students learning ARM Cortex-M register-level programming
- Developers transitioning from Arduino/HAL-based development to bare-metal STM32 programming
- Anyone using a NUCLEO-F411RE board who wants to understand GPIO internals

---

## ✨ Features

✅ Direct register-level GPIO configuration (no HAL/LL dependency)
✅ Enables the GPIOC peripheral clock via the RCC AHB1ENR register
✅ Configures a GPIO pin as output and reads another as input at the register level
✅ Software polling loop with a busy-wait delay for debouncing-style timing
✅ Toggles an LED output using XOR on the GPIO Output Data Register (ODR)
✅ Full STM32CubeIDE project structure — ready to import and build

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

## 📂 Folder Structure

```
01_Interfacing_LED_and_Buzzer_with_button/
│
├── .project                                        # Eclipse/STM32CubeIDE project descriptor
├── .cproject                                        # Build configuration (Debug/Release, compiler/linker options)
├── Interfacing_LED_and_Buzzer_with_button Debug.launch  # Debug launch configuration (ST-Link settings)
├── STM32F411RETX_FLASH.ld                           # Linker script — runs code from Flash
├── STM32F411RETX_RAM.ld                             # Linker script — runs code from RAM (debug-in-RAM)
│
├── Src/
│   ├── main.c                                        # Application entry point — GPIO button/LED logic
│   ├── syscalls.c                                    # Newlib syscall stubs (auto-generated)
│   └── sysmem.c                                       # Heap management for malloc/newlib (auto-generated)
│
└── Startup/
    └── startup_stm32f411retx.s                        # Reset handler, vector table, stack/data init (assembly)
```

> **Note:** As with the folder name, no buzzer-related code exists in `main.c` at this time — the current firmware only handles LED and button GPIO logic.

---

## ⚙️ How It Works

1. **Peripheral Clock Enable** — The program enables the GPIOC peripheral clock by setting bit 2 of the `RCC_AHB1ENR` register, since GPIOC must be clocked before its registers can be configured or used.
2. **Pin Configuration** — `GPIOC_MODER` is configured to set one pin as a general-purpose output (mode `01`), while the button pin is left in its default input state.
3. **Input Polling** — Inside an infinite `while(1)` loop, the program continuously reads the button pin's state from `GPIOC_IDR`.
4. **Debounce-Style Delay** — When the button is read as pressed, a software busy-wait loop (`delay()`, ~300,000 iterations) introduces a pause before acting.
5. **Output Toggle** — After the delay, the LED pin is toggled using an XOR operation on `GPIOC_ODR`, flipping its state each time the condition is met.
6. **Internal Workflow** — Because this check-delay-toggle sequence repeats continuously while the button remains pressed (rather than triggering once per press), the LED will keep toggling repeatedly for as long as the button stays held down.

---

## 🔍 Code Explanation

**`Src/main.c`**

| Element | Type | Purpose |
|---|---|---|
| `GPIOC_MODER` | Pointer to register (`0x40020800 + 0x00`) | Configures each GPIOC pin's mode (input/output/alternate/analog). |
| `GPIOC_ODR` | Pointer to register (`0x40020800 + 0x14`) | Output Data Register — controls the logic level driven on output pins. |
| `GPIOC_IDR` | Pointer to register (`0x40020800 + 0x10`) | Input Data Register — reflects the current logic level on each pin. |
| `AHB1_RCC_GPIOCEN` | Pointer to register (`0x40023800 + 0x30`) | RCC AHB1 peripheral clock enable register; bit 2 enables the GPIOC clock. |
| `delay()` | Function | A `volatile` busy-wait loop (~300,000 cycles) used as a crude timing delay. |
| `main()` | Function | Enables the GPIOC clock, configures pin modes, then polls the button pin (read via `GPIOC_IDR`, bit 0) and toggles the LED pin (`GPIOC_ODR`, bit 1) via XOR whenever the button is read as pressed (`value == 0`). |

**Pin mapping (as used in the register logic):**
- **Button input:** GPIOC pin 0 (read via `GPIOC_IDR` bit 0)
- **LED output:** GPIOC pin 1 (driven via `GPIOC_ODR` bit 1, configured as output via `GPIOC_MODER` bit 2)

**A note on the `MODER` configuration:**
The code clears bits `[1:0]` of `GPIOC_MODER` (`&= ~(3 << 0)`) before setting bit 2 (`|= (1<<2)`) to configure pin 1 as output. Since each GPIO pin occupies 2 bits in `MODER`, pin 1's field is actually bits `[3:2]` — meaning the clear operation targets pin 0's field rather than pin 1's. This happens to work correctly here only because `GPIOC_MODER` resets to `0x00000000` by default, so pin 1's field was already clear. It's functionally harmless in this specific case, but worth being aware of if this code is reused or extended, since the clear mask doesn't match the pin actually being configured.

**Other project files** (`syscalls.c`, `sysmem.c`, `startup_stm32f411retx.s`, the `.ld` linker scripts, `.project`, `.cproject`) are STM32CubeIDE-generated boilerplate handling the C runtime startup, memory layout, and IDE build/debug configuration — they are not modified application logic.

---

## ⚠️ Technical Note: Unexpected LED Behavior

While testing this project, an interesting hardware interaction was observed with a 2-pin RGB LED:

Because the polling loop repeatedly toggles the LED output pin for as long as the button is held (rather than toggling once per press), and mechanical button contact bounce adds further rapid, unintended transitions, the LED pin sees a burst of very fast power cycling rather than a single clean toggle. A 2-pin RGB LED contains a small embedded IC that automatically cycles through preset colors, and that IC can interpret rapid power interruptions as a reset or mode-advance signal. In practice, this caused the LED to skip its normal startup color and jump into fast multi-color strobing instead of a simple on/off toggle.

This behavior originates from the combination of the software's repeated-toggle loop and unmitigated button bounce — it was not an intentionally designed feature of the firmware, but it can be treated as a usable side effect: holding the button produces a fast color-cycling strobe effect with a 2-pin RGB LED, which could be leveraged as an informal "multi-mode" visual indicator if the timing is deliberately tuned.

**If consistent, single-toggle-per-press behavior is what you want instead**, this would need to be fixed with proper software debouncing (e.g., detecting only a rising/falling edge instead of polling on every loop iteration) — see "Future Improvements" below.

---

## 💻 Installation

**1. Clone the repository**
```bash
git clone https://github.com/anshukumar146/github.git
cd github/NUCLEOF411RE/01_Interfacing_LED_and_Buzzer_with_button
```

**2. Install STM32CubeIDE**
Download and install from [st.com/en/development-tools/stm32cubeide.html](https://www.st.com/en/development-tools/stm32cubeide.html) if not already installed.

**3. Import the project**
- Open STM32CubeIDE
- Go to **File → Open Projects from File System...**
- Select the `01_Interfacing_LED_and_Buzzer_with_button` folder
- Click **Finish** to import

---

## ▶️ Running the Project

1. Wire the circuit: connect a button to **PC0** (with an appropriate pull-up/pull-down as needed) and an LED (with a current-limiting resistor) to **PC1**.
2. Connect the NUCLEO-F411RE board to your computer via USB (this also provides the onboard ST-Link programmer/debugger).
3. In STM32CubeIDE, select the project in the Project Explorer.
4. Click the **Build** icon (or **Project → Build Project**) to compile.
5. Click **Run → Debug** (or the Debug icon) to flash the firmware to the board using the included ST-Link debug launch configuration.
6. Once flashed, the program runs automatically — press the button to toggle the LED.

---

## 🧑‍💻 Usage

- **Press and hold the button** connected to PC0 → the LED on PC1 will begin toggling repeatedly (see the "Technical Note" above regarding RGB LED behavior specifically).
- **Release the button** → toggling stops and the LED holds its last state.

No serial output, UI, or additional configuration is required — all behavior is driven by GPIO state.


---

## 🖼️ Schematic diagram
</h1>R2 is an external pull-up holding PC0 HIGH when SW1 is open. It's only needed because PC0's internal pull-up isn't enabled in firmware — configure it via GPIOC_PUPDR, and R2 can be removed. </h1>

<img width="900" height="500" alt="Screenshot 2026-07-29 200917" src="https://github.com/user-attachments/assets/5bc77384-353a-4625-ae6d-f15953a0375a" />




---

## 🎥 Demo Video



https://github.com/user-attachments/assets/99a4e906-a114-4928-ab05-1190e999df30



---

## 📊 Results

- With a standard single-color LED, holding the button produces repeated toggling (rapid blinking) rather than a single on/off state change per press.
- With a 2-pin RGB LED specifically, this same rapid toggling — compounded by button contact bounce — causes the LED's internal IC to cycle through its built-in color/strobe patterns instead of behaving like a simple binary indicator (see "Technical Note" above).

---

## 🚀 Future Improvements

- Implement proper edge-detection debouncing (software or via `EXTI` interrupt) so the LED toggles exactly once per button press, rather than repeatedly while held
- Add the buzzer functionality implied by the folder name — e.g., drive a buzzer output alongside the LED
- Replace the busy-wait `delay()` with a timer-based delay (e.g., `SysTick`) for more precise, non-blocking timing
- Migrate to STM32 HAL or LL drivers as an alternate branch, to compare bare-metal vs. abstracted GPIO handling
- Add configurable pull-up/pull-down setup in software via `GPIOC_PUPDR` instead of relying entirely on external circuitry
- If the RGB color-cycling behavior is intentionally kept, document and tune the toggle timing deliberately so the "Multi-Mode" effect is reliable and reproducible rather than an incidental side effect of bounce

---

## 🎓 Learning Outcomes

By studying and building this project, you will learn:

- How to enable peripheral clocks via the RCC register before using any peripheral
- How to configure GPIO pin modes by directly manipulating the `MODER` register
- How to read and write GPIO pin states via the `IDR` and `ODR` registers
- The difference between bare-metal register programming and HAL-based abstraction
- How software polling loops and busy-wait delays behave in embedded firmware
- Why button contact bounce matters and how it can produce unexpected hardware behavior
- How STM32CubeIDE structures a project (linker scripts, startup assembly, syscalls, build configuration)

---

## 🧩 Skills Demonstrated

- Bare-Metal Embedded C Programming
- ARM Cortex-M Register-Level GPIO Control
- STM32 Peripheral Clock & Pin Configuration
- STM32CubeIDE Project Setup & ST-Link Debugging
- Hardware Behavior Analysis & Debugging (contact bounce, IC-level LED behavior)
- Embedded Systems Documentation

---

## 👤 Author

**Name:** Anshu Kumar
**GitHub:** [@anshukumar146](https://github.com/anshukumar146)
**LinkedIn:** _(add your LinkedIn URL here)_
**Email:** _(add your email here)_

---

## 📜 License

This project is licensed under the **MIT License**. Note that `syscalls.c`, `sysmem.c`, `startup_stm32f411retx.s`, and the `.ld` linker scripts are STM32CubeIDE/STMicroelectronics-generated boilerplate, licensed under terms provided by STMicroelectronics (see file headers).

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

- STMicroelectronics for STM32CubeIDE and the auto-generated project scaffolding (linker scripts, startup code, syscalls)
- The STM32F411xC/E Reference Manual for register address and bit-field documentation
- The embedded systems community for resources on bare-metal ARM Cortex-M programming

---

<div align="center">

⭐ If you found this repository helpful, consider giving it a star!

</div>
