// Aroma Shot Timer — display pod casing
// Construction: 2D side profile, linear-extruded across the width.
// This is easier to reason about and edit than rotated 3D primitives.
//
// Orientation for printing: back face (mount plane) DOWN on the bed.
// Mounting: VHB/foam tape on the flat back; optional 6x3mm magnets.

// ---------------- Parameters ----------------
wall      = 1.8;    // wall thickness
inner_h   = 14;     // cavity height (OLED + ESP32 + wiring stack)
inner_d   = 34;     // cavity depth at the bottom
inner_w   = 40;     // cavity width
tilt_deg  = 12;     // front face tilt: top leans toward the viewer

// OLED (0.96" SSD1306 module)
oled_pcb_w   = 27.0;
oled_view_w  = 25.4;  // visible window (glass)
oled_view_h  = 12.0;
oled_seat   = 2.0;     // PCB sits this far behind the front face

magnet_d  = 6.2;
magnet_t  = 3.0;

$fn = 64;

// ---------------- Derived ----------------
tilt     = tan(tilt_deg);          // depth growth per unit height
outer_d  = inner_d + wall;
outer_h  = inner_h + 2*wall;
outer_w  = inner_w + 2*wall;
front_shift = tilt * outer_h;       // how much the top front edge leans out

// ---------------- Side profile ----------------
// Coordinates: x = depth from the mount face, y = height from the bed.
// Back face is x=0 (flat, mounts to machine). Front face leans outward
// at the top so the display tilts up toward the user.
module case_profile() {
  polygon([
    [0, 0],                        // back-bottom
    [outer_d, 0],                  // front-bottom
    [outer_d + front_shift, outer_h],  // front-top (tilted out)
    [0, outer_h],                  // back-top
  ]);
}

module cavity_profile() {
  // inset by wall on front and top/bottom; back stays at x=wall
  polygon([
    [wall, wall],
    [outer_d + tilt*wall - oled_seat, wall],
    [outer_d + tilt*(outer_h - wall) - oled_seat, outer_h - wall],
    [wall, outer_h - wall],
  ]);
}

// ---------------- Window ----------------
// A rectangle on the tilted front face, cut straight through along the
// face normal. Placed near the vertical middle of the face.
module window_cut() {
  cx = outer_d + tilt*outer_h/2;              // approx face mid-depth
  rotate([0, -tilt_deg, 0])                  // align with the face normal
    translate([cx - oled_seat/2, outer_h/2, 0])
      translate([-(oled_view_w + 2)/2, -(oled_view_h + 2)/2, -wall])
        cube([oled_view_w + 2, oled_view_h + 2, wall + 2*oled_seat + 2]);
}

// ---------------- Mount features ----------------
// Magnet pockets: blind holes in the back face (mount plane).
module magnet_pockets() {
  for (my = [wall + magnet_d/2 + 2, outer_h - wall - magnet_d/2 - 2])
    translate([magnet_t/2, my, 0])
      rotate([0, 90, 0])
        cylinder(d = magnet_d, h = magnet_t + 1, center = true);
}

// USB-C cable slot in the bottom face.
module usb_cut() {
  translate([outer_d/2 + 2, -0.01, outer_w/2 - 6])
    cube([8, wall + 0.5, 12]);   // x=depth slot through bottom wall
}

// ---------------- OLED shelf ----------------
// Two ledges the OLED PCB rests against, behind the window.
module oled_shelf() {
  for (sx = [-(oled_pcb_w/2 - 2), oled_pcb_w/2 - 2])
    translate([outer_d + tilt*(outer_h/2) - oled_seat - 1.0,
               outer_h/2 - oled_view_h/2 - 3, outer_w/2 + sx])
      cube([2.5, 1.6, 3.2]);
}

// ---------------- Render ----------------
difference() {
  // extrude the side profile across the width (z = width axis)
  translate([0, 0, -outer_w/2])
    linear_extrude(outer_w)
      case_profile();

  translate([0, 0, -inner_w/2])
    linear_extrude(inner_w)
      cavity_profile();

  window_cut();
  magnet_pockets();
  usb_cut();
}

oled_shelf();
