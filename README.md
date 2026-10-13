# Plus Aroma ESE Pad — Shot Timer

A vibration-sensed shot timer for the Plus Aroma (or any vibratory-pump ESE pad
machine). An SW-420 vibration module taped near the pump detects the pump
running; a 0.96" SSD1306 OLED shows a live `M:SS.s` timer during the shot, then
the final time and a good/bad rating.

## Hardware

| Part | Notes |
|---|---|
| ESP32-C3 Super Mini | Any ESP32-C3 board works; adjust pins |
| 0.96" SSD1306 OLED (I2C, 0x3C) | 4 pins: VCC/GND/SDA/SCL. A 1.3" SSH1106 works too (needs U8g2 lib) |
| SW-420 vibration module | Get the variant with the sensitivity potentiometer |

### Wiring

```text
SW-420          ESP32-C3          OLED
 VCC  ──────────┐ 3V3 ┌──────────── VCC
 GND  ─────────┐│ GND ┌└──────────── GND
 DO   ──────────┼┼─┐   ││
                ││ │   │└── SDA  ← GPIO8
                ││ └───┼─── SCL  ← GPIO9
                └┴────DO─ GPIO2
```

**Power distribution:** the ESP32-C3 Super Mini has only a single 3V3 and a
single GND pin, but both the OLED and SW-420 need power. Solder the loads
as a star at the pin: twist the OLED's and SW-420's 3V3 wires together and
solder the bundle into the ESP32's 3V3 pad; same for the two GND wires.
Heat-shrink both joints. Combined draw is < 30 mA — a soldered bundle
handles this without issue. Give the SW-420's pair extra slack so the
sensor can be repositioned during sensitivity tuning.

**Signal wiring:** SW-420 DO → GPIO2, OLED SDA → GPIO8, SCL → GPIO9.

**Placement inside the case:** ESP32 flat against the inner back wall,
USB-C pointing down toward the bottom cable slot, hot-glued. SW-420 glued
beside it on the same back wall (this is the vibration-coupled face). OLED
slides into its printed seat gap last — connect its four wires before
fully seating it. See `case/README.md` for assembly order.

Power the ESP32 from USB (bench) or a 5V AC-DC module (HLK-PM01 / Mini560)
wired to the machine's switched mains. **Mains wiring is at your own risk:
unplug the machine, keep mains-side creepage/clearance, heat-shrink everything.**

Recommended extras: 100 nF ceramic + 470 µF electrolytic across 3V3 near the
OLED; silicone stranded 26 AWG wire (solid core fatigue-breaks under vibration);
3M VHB 4910 tape to mount the printed case.

## Firmware

- State machine: `IDLE → RUNNING → DONE` (see `src/main.cpp`).
- A shot is "running" after 500 ms of sustained vibration; it ends after
  800 ms of quiet. Triggers shorter than 2 s are ignored (door slams, knocks).
- The SW-420 vibration element is normally closed: DO idles LOW at rest and
  pulses HIGH on vibration. Any HIGH within the last 100 ms counts as
  vibration, which tolerates the sensor's duty cycle.
- Good-shot band defaults to 20–30 s (`GOOD_MIN_MS` / `GOOD_MAX_MS`).
- If your module clone inverts the output (DO idles HIGH), set
  `VIBRATION_ACTIVE_LEVEL` to `LOW` in `src/main.cpp` — the input pull
  resistor follows automatically.

### Tuning

Flash, open the serial monitor at 115200, and watch the `state=... vib=...`
log while running shots. If it false-triggers or misses:

- `VIB_ON_WINDOW_MS` / `VIB_OFF_WINDOW_MS` — detection persistence.
- `VIB_PULSE_WINDOW_MS` — how forgiving the pulse detector is.
- Move the sensor closer to the pump if detection is marginal.
- If it triggers constantly at rest, check the on-board potentiometer first
  (threshold trim), then verify `VIBRATION_ACTIVE_LEVEL` matches your module.

## Case

See `case/` for the parametric OpenSCAD pod (OLED window, ESP32 + sensor
cavity, magnet pockets, 12° display tilt). Print a cheap test copy first and
verify the OLED sits centered behind the window before the final print.

## Build

```
pio run -t upload   # or Arduino IDE: install Adafruit SSD1306 + GFX, select ESP32C3
pio device monitor
```
