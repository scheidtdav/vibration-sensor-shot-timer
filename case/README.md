# Case — OLED display pod (v2)

Two-part, magnetically mounted case for the shot timer.

## Files

- `shot-timer-case.scad` — parametric OpenSCAD model rendering both parts:
  - **Pod**: tilted display case holding the OLED, ESP32-C3 and SW-420
  - **Backing plate**: thin plate that sticks to the machine (VHB tape) and
    carries the counterpart magnets

## Magnetic mounting

Four 8×3 mm neodymium disc magnets total (2 per part):

1. Press two magnets into the **blind pockets in the pod's back face** —
   pockets are 3.0 mm deep in a 3.6 mm wall, so a 0.6 mm floor keeps them
   captive; add a drop of CA glue for permanence.
2. Press the two counterpart magnets into the **backing plate's pockets**
   (mind the polarity — they must attract the pod's magnets across the gap).
3. Stick the backing plate to the machine with 3M VHB 4910/300LSE tape.
   The Plus Aroma's panels are plastic, so the plate is the steel/magnet
   interface the pod grabs onto.

Pull the pod off for battery/USB flashing; snap it back on — the magnet
spacing is identical on both parts, so it self-locates.

## Printing

- Material: PETG (tolerates the machine's warmth better than PLA).
- 0.2 mm layers, 3 perimeters, 20% infill, **no supports needed**: the model
  renders in print orientation — pod back face down, 12° tilt within
  printable overhang.
- Render STL: `openscad -o case.stl shot-timer-case.scad` (both parts
  included; separate them in your slicer or comment out one render line).

## Assembly

1. **Test-fit first**: verify the OLED PCB (27×27 mm) slides into the seat
   gap between front wall and seat plate, glass centered behind the window.
   Adjust `oled_view_w/h` if your module's glass differs.
2. Seat the OLED against the seat plate; fix with two Kapton tabs or a
   small hot-glue blob on the PCB corners (never on the glass).
3. ESP32-C3 Super Mini behind the OLED; there is ~7 mm of remaining cavity
   depth for board + wiring, and the USB-C cable exits through the bottom
   slot.
4. **SW-420**: press against the inside of the **back wall** — the back
   wall is the mount face, so machine vibration couples directly into the
   sensor. Hot glue transmits vibration better than foam tape.
5. Magnets into the pockets, backing plate onto the machine, snap on.

## Tuning the model

All dimensions live in the parameters block at the top:

- `oled_view_w/h` — window opening vs. your module's actual glass
- `inner_d` — cavity depth if you need more wiring room
- `tilt_deg` — display angle (12° default)
- `magnet_d/t` — pocket size vs. your actual magnets

## Design notes

The v2 model uses one consistent coordinate frame (profile: x = height,
y = depth, z = width; back face on the y=0 plane) instead of rotated 3D
primitives, precisely because the v1 model produced disconnected,
floating geometry. Every front-face feature (window, seat plate) is placed
via the shared `on_front()` transform along the face normal, so the seat
plate is guaranteed parallel to the window and fused into the side walls
(it spans the full outer width). Magnet pockets are blind bores (depth
3.0 mm in a 3.6 mm wall) — the v1 pockets pierced straight through the
back wall.
