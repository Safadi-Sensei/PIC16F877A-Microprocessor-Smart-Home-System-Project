````markdown
# Light & Motion Control System

**Authors:**  
- Abdelrahman Alsafadi (64210108)  
- Zaid Abdelkarim (64210014)  
- Abdalrahman Shady (64210098)  

**Affiliation:**
Prof. Mustafa TÜRKBOYLARI
Istanbul Medipol University, Department of Computer Engineering

---

## 1. Project Overview

This project implements a **real-time light and motion control system** on a PIC16F877A microcontroller. It continuously monitors:

- **Ambient light** via an LDR (Analog input AN0)  
- **Motion** via a PIR sensor (Digital input RB0)  

It displays status on a 16×2 LCD and drives two indicator LEDs:

- **LIGHT_LED** indicates “dark” conditions  
- **MOTION_LED** indicates detected motion  

---

## 2. Features

- **4-bit LCD interface** (RD2–RD7)  
- **On-chip A/D converter** for the LDR  
- **PIR motion detector**  
- **Configurable light threshold** (default 50 %)  
- **Debounced LCD updates**—only redraws when state changes  
- **Startup splash & calibration messages**  
- **Fully commented XC8 C code**

---

## 3. Hardware Requirements

- **Microcontroller:** PIC16F877A  
- **Power:** 5 V regulated supply  
- **LCD:** Hitachi-compatible 16×2 (4-bit mode)  
- **Sensors & LEDs:**
  - LDR + voltage divider → RA0/AN0  
  - PIR sensor → RB0  
  - “Dark” LED → RB4  
  - “Motion” LED → RB2  
- **Programmer:** PICkit™ or ICD  
- **Misc:** Breadboard, jumper wires, bypass caps

---

## 4. Pin Mapping

| MCU Pin     | Signal       | Description                         |
| :---------: | :----------- | :---------------------------------- |
| RA0/AN0     | LDR input    | ADC channel 0 (ambient light)       |
| RB0          | PIR input   | Digital motion sensor               |
| RB2          | MOTION_LED  | Active-high motion indicator LED    |
| RB4          | LIGHT_LED   | Active-high “dark” indicator LED    |
| RD2          | LCD_RS      | LCD register-select                  |
| RD3          | LCD_EN      | LCD enable                          |
| RD4–RD7      | LCD_D4–D7   | LCD data bus (4-bit mode)           |

---

## 5. Configuration Bits

```c
#pragma config FOSC = HS        // External high-speed oscillator
#pragma config WDTE = OFF       // Watchdog timer disabled
#pragma config PWRTE = ON       // Power-up timer enabled
#pragma config BOREN = ON       // Brown-out reset enabled
#pragma config LVP  = OFF       // Low-voltage programming disabled
#pragma config CPD  = OFF       // Data EEPROM code protection off
#pragma config WRT  = OFF       // Flash write protection off
#pragma config CP   = OFF       // Code protection off

#define _XTAL_FREQ 20000000     // 20 MHz system clock for delays


---

## 6. Software Setup

1. **Install MPLAB X IDE** (v5.50 or later).
2. **Install XC8 Compiler** (v2.30 or later).
3. Create a new **“Standalone Project”** for PIC16F877A.
4. Add **`MicroProjectCode.c`** to the project’s source folder.
5. In Project Properties → XC8 Compiler → ensure \_XTAL\_FREQ and config bits match your hardware.

---

## 7. Build & Program

1. **Build** the project (Project ▶ Clean & Build).
2. Connect your PICkit™ or ICD programmer.
3. **Program** the compiled `.hex` into the PIC16F877A.
4. Reset the board; the startup and calibration screens will appear on the LCD.

---

## 8. Usage

* After calibration, the LCD enters the main loop:

  * **Line 1:** `L:xx%  M:YES/NO`
  * **Line 2:** `LT:ON/OFF  MT:ON/OFF`
* Change the ambient light or introduce motion to see LED and LCD updates.
* To adjust the **light threshold**, modify the comparison in `main()`:

  ```c
  if (light_percentage < YOUR_THRESHOLD) {
      LIGHT_LED = 1;
  } else {
      LIGHT_LED = 0;
  }
  ```

---

## 9. Project Structure

```
/
├─ MicroProjectCode.c      # Main source code
└─ README.md               # Project documentation
```

---

## 10. Customization

* **Sensor Channels:** Use `ADC_Read(channel)` to monitor other analog inputs.
* **Display Layout:** Edit `LCD_String()` and `LCD_Goto()` calls to change messages.
* **Timing:** Tweak `__delay_ms()` values for different refresh rates.
* **Add Features:** e.g., buzzer alerts, UART logging, or SD-card data logging.
