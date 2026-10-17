# TouchForge — ESP32 Touch Reaction Game

> **Important project note:** The supplied Arduino sketch is titled **“TouchForge — Experiment 4: F1 Reaction Finder (v2 + calibration)”**. It implements a touch-based reaction-time game, **not a Chrome-style Dino runner game**. The provided schematic and sketch have been included as supplied project materials. If the goal is specifically a Dino runner game, the game logic and likely the display wiring/code will need to be changed.

## Overview

TouchForge is an ESP32-based reaction-time game. The player starts a countdown by touching a capacitive touch sensor, waits for the signal, then touches again as quickly as possible when the signal changes. The sketch displays reaction time and the best result on a 128×64 OLED.

### Features in the supplied sketch

- Touch input starts a new round.
- 3–2–1–START countdown with sound feedback.
- Random waiting interval before the reaction signal.
- False-start detection.
- Reaction-time measurement and best-time tracking during the current power session.
- OLED status screens and buzzer feedback.
- Startup calibration that checks for spurious touch-sensor activity.

## Repository layout

```text
TouchForge-Dino-Game-Repository/
├── README.md
├── wiring-schematic.png
├── docs/
│   └── wiring-schematic.png
└── src/
    └── TouchForge_F1_Reaction_Finder.ino
```

## Hardware shown / referenced

- ESP32 development board (ESP-WROOM-32 style board in the schematic)
- Touch sensor module
- 128×64 I²C OLED display (SSD1306-compatible)
- Buzzer module
- LED
- Breadboard and jumper wires

The sketch also defines the following GPIO assignments:

| Function | GPIO / setting in sketch |
|---|---:|
| Touch sensor digital output | GPIO 4 |
| LED | GPIO 2 |
| Buzzer | GPIO 18 |
| OLED SDA | GPIO 21 (`Wire.begin(21, 22)`) |
| OLED SCL | GPIO 22 (`Wire.begin(21, 22)`) |
| OLED I²C address | `0x3C` |

**Wiring caution:** Please verify the schematic against these GPIO assignments before powering the circuit. The uploaded drawing may not clearly match every pin assignment in the sketch. Power the modules according to their own specifications and the ESP32 board’s logic-level requirements.

## Software requirements

1. Arduino IDE 2.x (or a compatible Arduino IDE).
2. ESP32 board support installed through Boards Manager.
3. Libraries:
   - **Adafruit GFX Library**
   - **Adafruit SSD1306**

`Wire.h` is included with the Arduino platform. The sketch uses ESP32 LEDC tone functions, so select an ESP32 board/core compatible with `ledcAttach()` and `ledcWriteTone()` (the sketch is written for the newer ESP32 Arduino core API).

## Setup and upload

1. Open `src/TouchForge_F1_Reaction_Finder.ino` in Arduino IDE.
2. Install **Adafruit GFX Library** and **Adafruit SSD1306** from Library Manager. Install any missing dependency requested by Library Manager.
3. Select the correct ESP32 board and COM port.
4. Connect the OLED to the I²C pins configured in the sketch (SDA GPIO 21, SCL GPIO 22), and connect the touch sensor, LED, and buzzer to the GPIOs listed above.
5. Verify the schematic and wiring carefully before powering the board.
6. Click **Verify**, then **Upload**.
7. Open Serial Monitor at **115200 baud** for calibration messages and reaction-time logs.
8. During startup calibration, do not touch the sensor.

## How to play

1. Touch the sensor to begin.
2. Wait through the 3–2–1 countdown and the START/READY sequence.
3. Do not touch during the waiting period; doing so triggers a false start.
4. Touch when the display clears / the signal is given.
5. Read the reaction time and best time on the OLED.
6. Touch again when the result screen allows another round.

## Troubleshooting

- **OLED not found:** Check SDA/SCL, I²C address (`0x3C`), power, and ground.
- **False starts without touching:** Keep the touch pad away from wires/metal, adjust the sensor module’s sensitivity trimmer if available, and check grounding.
- **Buzzer does not sound:** Verify GPIO 18 and the module’s polarity/interface.
- **LED does not turn on:** Verify GPIO 2 and the LED’s current-limiting resistor.
- **Compilation error around `ledcAttach()` or `ledcWriteTone()`:** Check the installed ESP32 board package version. These calls depend on the ESP32 Arduino core API; older cores may require different LEDC setup code.
- **Serial upload issue:** Close Serial Monitor and other applications using the COM port, then reconnect the board and select the port again.

## Project files

- [`src/TouchForge_F1_Reaction_Finder.ino`](src/TouchForge_F1_Reaction_Finder.ino) — Arduino sketch from the supplied source.
- [`wiring-schematic.png`](wiring-schematic.png) — supplied wiring diagram.
- [`docs/wiring-schematic.png`](docs/wiring-schematic.png) — duplicate copy for documentation.

## License

No license was provided with the source files. Add a license before granting others permission to reuse or redistribute this project.
