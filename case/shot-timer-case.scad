// Aroma Shot Timer — display pod casing (v2, complete redesign)
//
// Construction: ONE consistent coordinate frame throughout.
//   Profile frame: x = height, y = depth, z = width.
//     - Back face (mount plane): the plane y = 0, flat and vertical.
//     - Bottom face: the plane x = 0.
//     - Front face: tilted by tilt_deg, leaning OUT at the top.
//   The final render applies rotate([90,0,0]) so the flat back face
//   lies on the print bed — that is the intended print orientation,
//   no supports needed (12 deg overhang).
//
// Mounting: magnetic. Two 8x3mm magnets in the pod's back face grip
// the machine's steel panels directly.

// ---------------- Parameters ----------------
wall      = 3.6;    // wall thickness (thick enough for blind magnet pockets)
inner_h   = 30;     // cavity height — must fit the 27mm OLED PCB upright
inner_d   = 20;     // cavity depth: ESP32 + perfboard bus + Dupont jumpers
                    // (lying on their side) fit behind the seat plate
inner_w   = 36;     // cavity width — PCB 27mm sits with margin
tilt_deg  = 12;     // front face tilt: top leans toward the viewer

// OLED (0.96" SSD1306 module, 27x27mm PCB, mounted parallel to front face)
oled_pcb    = 27.0;
oled_view_w = 24.0;   // window width  (measure your module's glass!)
oled_view_h = 12.0;   // window height (glass is ~11mm tall at size-3 text)
oled_seat   = 2.4;    // gap between inner front face and the seat plate (PCB thickness + slop)
plate_t     = 2.0;    // seat plate thickness

// Magnets: 8x3mm neodymium discs, pressed into the pod's back pockets
// and gripping the machine's steel panels directly
magnet_d = 8.2;
magnet_t = 3.0;

$fn = 64;

// ---------------- Derived ----------------
shift   = tan(tilt_deg);            // front lean per unit height
outer_h = inner_h + 2*wall;
outer_d = inner_d + wall;          // front wall counted via the offset below
front_shift = shift * outer_h;     // how far the top front edge leans out
L  = sqrt(outer_h*outer_h + front_shift*front_shift); // front face length

// ---------------- Pod body ----------------
// Extruded side profile: x = height (0..outer_h), y = depth (0..outer_d).
module pod_profile() {
  polygon([
    [0, 0],                         // back-bottom
    [outer_h, 0],                   // back-top
    [outer_h, outer_d + front_shift],  // front-top (tilted out)
    [0, outer_d],                   // front-bottom
  ]);
}

module cavity_profile() {
  // Inner front face = outer front face offset inward by `wall`
  // (exactly parallel: same slope, shifted by wall in -y).
  polygon([
    [wall, wall],
    [outer_h - wall, wall],
    [outer_h - wall, inner_d + front_shift*(outer_h - wall)/outer_h],
    [wall, inner_d],
  ]);
}

module pod_body() {
  difference() {
    union() {
      translate([0, 0, -inner_w/2 - wall])
        linear_extrude(inner_w + 2*wall) pod_profile();
      oled_seat_plate();
    }
    translate([0, 0, -inner_w/2])
      linear_extrude(inner_w) cavity_profile();
    window_cut();
    magnet_pockets();
    usb_slot();
  }
}

// ---------------- Front-face helpers ----------------
// All front-face features share this transform: a point on the face at
// height xh, offset inward along the face normal by `off`.
// Outward face normal (profile frame): (sin(tilt), -cos(tilt)) i.e. the
// front face leans out, so inward = (-sin, +cos) direction.
module on_front(xh, off) {
  ang = tilt_deg;
  y_face = outer_d + front_shift * xh / outer_h;
  nx = -sin(ang);   // inward normal, x component
  ny = cos(ang);    // inward normal, y component
  translate([xh + nx*off, y_face + ny*off, 0])
    rotate([0, 0, ang])
      children();
}

// Window: rectangular cut through the front wall, along the face normal.
// Cube is thin in local y (depth); rotate([0,0,tilt]) aligns its -y face
// with the outward face normal.
module window_cut() {
  on_front(outer_h/2, 0)
    cube([oled_view_h + 2, 2*wall + 4, oled_view_w + 2], center = true);
}

// OLED seat plate: a full-width plate `oled_seat` behind the inner front
// face, with a window-sized hole. The PCB slides into the gap between
// plate and front wall, glass toward the window. The plate spans the
// full cavity width PLUS 2*wall so it fuses into the side walls —
// no floating parts.
module oled_seat_plate() {
  difference() {
    on_front(outer_h/2, wall + oled_seat + plate_t/2)
      cube([oled_pcb, plate_t, inner_w + 2*wall], center = true);
    on_front(outer_h/2, wall + oled_seat + plate_t/2)
      cube([oled_view_h + 2, plate_t + 4, oled_view_w + 2], center = true);
  }
}

// ---------------- Back-face mount features ----------------
// Blind magnet pockets drilled into the back face (y = 0 plane),
// axis along +y. Pocket depth < wall, so a floor remains.
module magnet_pockets() {
  for (mx = [wall + magnet_d/2 + 1, outer_h - wall - magnet_d/2 - 1])
    translate([mx, -0.05, 0])
      rotate([-90, 0, 0])
        cylinder(d = magnet_d, h = magnet_t);
}

// USB-C cable slot through the bottom face (x = 0 plane), near the front.
module usb_slot() {
  translate([-0.1, outer_d - 11, -6])
    cube([wall + 0.2, 9, 12]);
}

// ---------------- Render (print orientation) ----------------
// rotate([90,0,0]) puts the pod's flat back face (y=0) onto the bed (z=0):
// the tilted front face points up at 12 deg — printable without supports.
// The pod's magnets grab the machine's steel panels directly.
rotate([90, 0, 0]) pod_body();
