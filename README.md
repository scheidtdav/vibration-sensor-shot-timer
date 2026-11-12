# Plus Aroma ESE Pad — Shot Timer

A vibration-sensed shot timer for the Plus Aroma (or any vibratory-pump ESE pad
machine). An SW-420 vibration module near the pump detects the pump running;
a 0.96" SSD1306 OLED shows a live `M:SS.s` timer during the shot, then the
final time and a good/bad rating.

> **Note:** This project was entirely vibe coded.

## Hardware

| Part | Notes |
|---|---|
| ESP32-C3 Super Mini | Any ESP32-C3 board works; adjust pins |
| 0.96" SSD1306 OLED (I2C, 0x3C) | 4 pins: VCC/GND/SDA/SCL |
| SW-420 vibration module | Get the variant with the sensitivity potentiometer |

### Wiring

```text
SW-420          ESP32-C3          OLED
 VCC  ─────────┐ 3V3 ┌──────────── VCC
 GND  ────────┐│ GND ┌└──────────── GND
 DO   ─────────┼┼─┐   ││
                ││ │   │└── SDA  ← GPIO8
                ││ │   └─── SCL  ← GPIO9
                └┴─┴────── DO── GPIO2
```

The ESP32-C3 Super Mini has only one 3V3 and one GND pin, but both the OLED
and SW-420 need power. Route the loads through a small perfboard power bus
(one bridged strip per rail, header pins, Dupont jumpers) — or solder the
wires as a twisted bundle directly onto the pins if you don't mind the
rework pain. Give the SW-420 some slack so it can be repositioned during
tuning.

Power the ESP32 from USB, or from a 5V AC-DC module (HLK-PM01) wired to the
machine's switched mains. **Mains wiring is at your own risk.**

## Firmware

- State machine: `IDLE → RUNNING → DONE` (see `src/main.cpp`).
- A shot is "running" after 500 ms of sustained vibration; it ends after
  800 ms of quiet. Triggers shorter than 2 s are ignored.
- The SW-420 element is normally closed: DO idles LOW at rest, pulses HIGH
  on vibration (`VIBRATION_ACTIVE_LEVEL`).
- Good-shot band defaults to 20–30 s (`GOOD_MIN_MS` / `GOOD_MAX_MS`).

### Tuning

Watch the `state=... vib=...` serial log at 115200 while running shots. If
it false-triggers or misses, trim the on-board potentiometer first, then
adjust `VIB_ON_WINDOW_MS` / `VIB_OFF_WINDOW_MS` / `VIB_PULSE_WINDOW_MS`.

## Build

```
pio run -t upload   # or Arduino IDE: install Adafruit SSD1306 + GFX, select ESP32C3
pio device monitor
```
