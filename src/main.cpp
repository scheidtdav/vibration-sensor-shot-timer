#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>

// ---------------- Pin map (ESP32-C3 Super Mini) ----------------
constexpr int PIN_VIBRATION_IN = 2;   // SW-420 DO -> GPIO2
constexpr int PIN_SDA          = 8;   // OLED SDA
constexpr int PIN_SCL          = 9;   // OLED SCL

// ---------------- OLED ----------------
constexpr int SCREEN_WIDTH  = 128;
constexpr int SCREEN_HEIGHT = 64;
constexpr int OLED_I2C_ADDR = 0x3C;
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---------------- Tunables ----------------
constexpr uint32_t VIB_ON_WINDOW_MS   = 500;   // sustained vibration before a shot is "running"
constexpr uint32_t VIB_OFF_WINDOW_MS  = 800;   // sustained quiet before a shot "ends"
constexpr uint32_t MIN_SHOT_MS        = 2000;  // ignore triggers shorter than this (knocks, door slams)
constexpr uint32_t RESULTS_MS         = 10000; // show result screen this long, then reset
constexpr uint32_t DISPLAY_PERIOD_MS  = 100;   // display refresh interval while running

constexpr uint32_t GOOD_MIN_MS = 20000;  // ESE pad: good shot band 20-30 s
constexpr uint32_t GOOD_MAX_MS = 30000;

// ---------------- State machine ----------------
enum class ShotState { IDLE, RUNNING, DONE };

struct Vibration {
  uint32_t lastChangeMs  = 0;
  uint32_t activeSinceMs = 0;  // 0 = not active
  bool     currentlyVibrating = false;
  uint32_t lastVibMs = 0;      // last time vibration was observed (any duration)
};

static ShotState state = ShotState::IDLE;
static Vibration vib;
static uint32_t shotStartMs  = 0;
static uint32_t shotEndMs    = 0;
static uint32_t lastDrawMs  = 0;

static uint32_t shotCount = 0;
static uint32_t lastShotMs = 0;

// ---------------- Detection ----------------
// The SW-420 vibration element is NORMALLY CLOSED: at rest the circuit is
// closed and the module's DO idles LOW (comparator output near GND). When
// vibration shakes the switch open, DO pulses HIGH. We therefore treat "any
// HIGH within the last 100 ms" as vibrating, which forgives the sensor's
// own duty cycle.
//
// NOTE: some clone modules invert this (DO idles HIGH, pulses LOW). If your
// sensor triggers constantly at rest, flip ACTIVE_LEVEL below to LOW.
constexpr int VIBRATION_ACTIVE_LEVEL = HIGH;  // DO pulses HIGH on vibration
constexpr uint32_t VIB_PULSE_WINDOW_MS = 100;

static void pollVibration() {
  static uint32_t lastSampleMs = 0;
  uint32_t now = millis();

  if (now - lastSampleMs < 5) return;  // 200 Hz sampling is plenty
  lastSampleMs = now;

  bool rawActive = (digitalRead(PIN_VIBRATION_IN) == VIBRATION_ACTIVE_LEVEL);
  if (rawActive) vib.lastVibMs = now;
  vib.currentlyVibrating = (now - vib.lastVibMs) < VIB_PULSE_WINDOW_MS;
}

// ---------------- Rendering ----------------
static void drawCenteredText(const char* text, int16_t y, int16_t size) {
  display.setTextSize(size);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, y, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, y);
  display.print(text);
}

// Draw a small espresso cup with saucer and steam, centered on screen.
// Pure pixel drawing, no bitmap data needed.
static void drawEspressoCup(int16_t cx, int16_t cy) {
  // --- cup body ---
  // trapezoid: wide at the rim (top), narrower at the base
  display.fillTriangle(cx - 12, cy - 10, cx + 12, cy - 10,
                       cx - 6, cy + 4, SSD1306_WHITE);
  display.fillTriangle(cx + 12, cy - 10, cx - 6, cy + 4,
                       cx + 6, cy + 4, SSD1306_WHITE);
  // hollow it out so the cup has walls (espresso "crema" stays visible)
  display.fillTriangle(cx - 9, cy - 9, cx + 9, cy - 9,
                       cx - 4, cy + 2, SSD1306_BLACK);
  display.fillTriangle(cx + 9, cy - 9, cx - 4, cy + 2,
                       cx + 4, cy + 2, SSD1306_BLACK);
  // re-add a crema line at the top of the coffee
  display.fillRect(cx - 8, cy - 9, 16, 2, SSD1306_WHITE);

  // --- handle ---
  // a ring to the right of the cup; it just touches the cup wall,
  // which reads as an attached handle
  display.fillCircle(cx + 14, cy - 4, 4, SSD1306_WHITE);
  display.fillCircle(cx + 14, cy - 4, 2, SSD1306_BLACK);

  // --- saucer ---
  display.fillTriangle(cx - 16, cy + 5, cx + 16, cy + 5,
                       cx - 8, cy + 9, SSD1306_WHITE);
  display.fillTriangle(cx + 16, cy + 5, cx - 8, cy + 9,
                       cx + 8, cy + 9, SSD1306_WHITE);
  // thin line between cup base and saucer
  display.drawFastHLine(cx - 7, cy + 5, 14, SSD1306_WHITE);
}

static void renderIdle() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  drawEspressoCup(SCREEN_WIDTH / 2, 24);          // cup in the upper half
  drawCenteredText("SHOT TIMER", 44, 2);  // message centered beneath it
  display.display();
}

static void renderRunning(uint32_t elapsedMs) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  uint32_t ds   = (elapsedMs / 100) % 10;
  uint32_t sec  = (elapsedMs / 1000) % 60;
  uint32_t mins = elapsedMs / 60000;

  char buf[16];
  snprintf(buf, sizeof(buf), "%lu:%02lu.%lu", (unsigned long)mins,
           (unsigned long)sec, (unsigned long)ds);
  drawCenteredText(buf, 16, 3);

  // progress bar for the good-shot band
  float frac = (float)elapsedMs / (float)GOOD_MAX_MS;
  if (frac > 1.0f) frac = 1.0f;
  display.drawRect(0, 56, SCREEN_WIDTH, 8, SSD1306_WHITE);
  display.fillRect(2, 58, (int)((SCREEN_WIDTH - 4) * frac), 4, SSD1306_WHITE);
  display.display();
}

static void renderDone(uint32_t shotMs) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  uint32_t sec  = (shotMs / 1000) % 60;
  uint32_t mins = shotMs / 60000;

  char buf[16];
  snprintf(buf, sizeof(buf), "%lu:%02lu", (unsigned long)mins, (unsigned long)sec);
  drawCenteredText("SHOT", 0, 1);
  drawCenteredText(buf, 16, 3);

  bool good = (shotMs >= GOOD_MIN_MS && shotMs <= GOOD_MAX_MS);
  drawCenteredText(good ? "GOOD SHOT" : "OUT OF BAND", 48, 1);

  char count[24];
  snprintf(count, sizeof(count), "Shot #%lu", (unsigned long)shotCount);
  drawCenteredText(count, 40, 1);
  display.display();
}

// ---------------- State transitions ----------------
static void toRunning() {
  state = ShotState::RUNNING;
  shotStartMs = millis();
  shotCount++;
  lastDrawMs = 0;
}

static void toDone() {
  state = ShotState::DONE;
  shotEndMs = millis();
  lastShotMs = shotEndMs - shotStartMs;
  renderDone(lastShotMs);
}

// ---------------- App ----------------
static void logStatus() {
  Serial.print("state=");
  Serial.print(state == ShotState::IDLE ? "IDLE" :
               state == ShotState::RUNNING ? "RUNNING" : "DONE");
  Serial.print(" vib=");
  Serial.println(vib.currentlyVibrating ? 1 : 0);
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_VIBRATION_IN, VIBRATION_ACTIVE_LEVEL == HIGH ? INPUT_PULLDOWN : INPUT_PULLUP);

  Wire.begin(PIN_SDA, PIN_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    Serial.println("SSD1306 init failed");
  }
  display.clearDisplay();
  renderIdle();

  Serial.println("Aroma shot timer ready");
}

void loop() {
  pollVibration();
  uint32_t now = millis();

  switch (state) {
    case ShotState::IDLE: {
      if (vib.currentlyVibrating) {
        if (vib.activeSinceMs == 0) vib.activeSinceMs = now;
        if (now - vib.activeSinceMs >= VIB_ON_WINDOW_MS) toRunning();
      } else {
        vib.activeSinceMs = 0;
      }
      break;
    }
    case ShotState::RUNNING: {
      if (!vib.currentlyVibrating && (now - vib.lastVibMs) >= VIB_OFF_WINDOW_MS) {
        if (now - shotStartMs < MIN_SHOT_MS) {
          state = ShotState::IDLE;  // false trigger; don't count it
          renderIdle();
          break;
        }
        toDone();
        break;
      }
      if (now - lastDrawMs >= DISPLAY_PERIOD_MS) {
        lastDrawMs = now;
        renderRunning(now - shotStartMs);
      }
      break;
    }
    case ShotState::DONE: {
      if (now - shotEndMs >= RESULTS_MS || vib.currentlyVibrating) {
        state = ShotState::IDLE;
        renderIdle();
      }
      break;
    }
  }

  static uint32_t lastLogMs = 0;
  if (Serial.available() == 0 && millis() - lastLogMs > 1000) {
    lastLogMs = millis();
    logStatus();
  }
}
