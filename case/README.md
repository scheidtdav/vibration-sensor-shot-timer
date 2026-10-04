# Case — OLED display pod

3D-printable pod for the shot timer: OLED behind a front window, ESP32-C3 and
SW-420 inside, mounted to the machine's side or front panel.

## Files

- `shot-timer-case.scad` — parametric OpenSCAD model. Edit the parameters at
  the top (wall thickness, cavity size, tilt, magnet size) and render.

## Printing

- Material: PETG or PLA (PETG better next to a warm machine).
- 0.2 mm layers, 3 perimeters/walls, no supports needed if printed with the
  mount face down (the wedge is only `tilt_deg` = 12°, within printable
  overhang).
- Slicer: render with OpenSCAD (`F6` → export STL), or
  `openscad -o case.stl shot-timer-case.scad`.

## Assembly

1. Test-fit: print once, check the OLED PCB sits behind the window with its
   glass centered. Adjust `oled_view_w/h` or the ledge height if needed.
2. Seat the OLED against the internal ledge tabs, secure with a small blob
   of hot glue or two Kapton tabs on the PCB corners (not on the glass).
3. ESP32-C3 Super Mini in the cavity behind the OLED, USB-C aligned with the
   bottom cutout.
4. SW-420 module: press it against the inside of the **back wall** with a
   foam pad — the back wall is the mount face, so pump vibration couples
   directly into it. This is the sensor's sweet spot; if detection is
   marginal, add a drop of hot glue between module and back wall (hot glue
   transmits vibration far better than foam tape).
5. Two 6×3 mm magnets in the back pockets (friction fit, or a drop of CA),
   then stick to the machine — or skip magnets and use 3M VHB tape on the
   flat back.

## Mounting on the Plus Aroma

Best spots: flat side panel near the pump (strongest vibration coupling) or
the front, angled toward the user by the case's 12° wedge. Magnets need a
steel surface — the Aroma's panels are plastic, so VHB foam tape is the
primary mount; magnets are for steel-bodied machines or a stuck-on steel
washer.

## Notes

- The model is parametric and unverified against a specific OLED module's
  glass position — do one cheap test print before the final one.
- `oled_view_h` (12 mm) shows roughly the middle band of the 64-pixel screen
  at text size 3 (24 px ≈ 9.5 mm); widen the window if you want the full
  display visible.
