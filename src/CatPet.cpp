#include "CatPet.h"

#include <math.h>

// ---- palette indices (8-bit sprite) ----
static const uint8_t IDX_WALL = 0;
static const uint8_t IDX_FLOOR = 1;
static const uint8_t IDX_LINE = 2;
static const uint8_t IDX_CAT_BODY = 3;
static const uint8_t IDX_PINK = 4;
static const uint8_t IDX_BLACK = 5;
static const uint8_t IDX_WHITE = 6;
static const uint8_t IDX_PLATE = 7;
static const uint8_t IDX_FOOD = 8;
static const uint8_t IDX_CAT_STRIPE = 9;
static const uint8_t IDX_PLANT_POT = 10;
static const uint8_t IDX_PLANT_LEAF = 11;
static const uint8_t IDX_RUG = 12;
static const uint8_t IDX_RUG_PATTERN = 13;
static const uint8_t IDX_WINDOW_FRAME = 14;
static const uint8_t IDX_WINDOW_SKY = 15;
static const uint8_t IDX_WINDOW_HILL = 16;
static const uint8_t IDX_SHADOW = 17;
static const uint8_t IDX_HEART = 18;
static const uint8_t IDX_ZZZ = 19;
static const uint8_t IDX_WINDOW_SUN = 20;
static const uint8_t IDX_STAR = 21;

// ---- scene geometry ----
static const int SCR_W = 320;
static const int SCR_H = 240;
static const int FLOOR_Y = 196;   // where wall meets floor; cat feet rest here
static const int PLATE_CX = 286;  // food plate center x

// ---- cat sprite ----
static const int PX = 3;          // size of one art-pixel
static const int BODY_W = 16;     // art-pixels wide
static const int BODY_ROWS = 11;  // body/head rows (legs drawn separately)
static const int LEG_ROWS = 3;
static const int CAT_PIX_H = (BODY_ROWS + LEG_ROWS) * PX;  // 42px

// Detailed pastel tabby silhouette:
// 0: transparent, 1: body (cream), 2: pink (ears, cheeks, nose), 3: stripes, 4: white chest
static const uint8_t BODY_MAP[BODY_ROWS][BODY_W] = {
    {  0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0 },
    {  0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0 },
    {  0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0 },
    {  0, 1, 1, 1, 1, 3, 3, 1, 1, 3, 3, 1, 1, 1, 1, 0 },
    {  1, 1, 1, 1, 1, 1, 3, 1, 1, 3, 1, 1, 1, 1, 1, 1 },
    {  3, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 3 },
    {  1, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 3, 2, 1 },
    {  1, 1, 1, 1, 1, 4, 4, 2, 2, 4, 4, 1, 1, 1, 1, 1 },
    {  0, 3, 3, 1, 4, 4, 4, 4, 4, 4, 4, 4, 1, 3, 3, 0 },
    {  0, 1, 3, 3, 1, 4, 4, 4, 4, 4, 4, 1, 3, 3, 1, 0 },
    {  0, 0, 1, 1, 1, 1, 4, 4, 4, 4, 1, 1, 1, 1, 0, 0 },
};

// Looping behaviour script. Walk steps carry a target x; the rest carry a
// duration in ms.
struct ScriptStep {
  CatPet::Action action;
  int arg;
};
// NOTE: kept in the .cpp; Action is private but this struct is file-local.
static const ScriptStep SCRIPT[] = {
    {CatPet::Action::Walk, 55},
    {CatPet::Action::Sit, 2600},
    {CatPet::Action::Walk, 245},  // head to the food plate
    {CatPet::Action::Eat, 4200},
    {CatPet::Action::Walk, 150},
    {CatPet::Action::Play, 2800},
    {CatPet::Action::Sit, 2200},
    {CatPet::Action::Walk, 270},
    {CatPet::Action::Sit, 1800},
};
static const int SCRIPT_LEN = sizeof(SCRIPT) / sizeof(SCRIPT[0]);

static const float WALK_SPEED = 2.2f;
static const uint32_t BLINK_INTERVAL = 3200;
static const uint32_t BLINK_LEN = 160;

void CatPet::applyDayPalette() {
  canvas_.setPaletteColor(IDX_WALL,         255, 232, 218);
  canvas_.setPaletteColor(IDX_FLOOR,        244, 215, 180);
  canvas_.setPaletteColor(IDX_LINE,         198, 172, 162);
  canvas_.setPaletteColor(IDX_CAT_BODY,     255, 246, 230);
  canvas_.setPaletteColor(IDX_PINK,         255, 180, 185);
  canvas_.setPaletteColor(IDX_BLACK,         70,  55,  50);
  canvas_.setPaletteColor(IDX_WHITE,        255, 255, 255);
  canvas_.setPaletteColor(IDX_PLATE,        235, 245, 230);
  canvas_.setPaletteColor(IDX_FOOD,         240, 145,  95);
  canvas_.setPaletteColor(IDX_CAT_STRIPE,   242, 180, 128);
  canvas_.setPaletteColor(IDX_PLANT_POT,    235, 150, 120);
  canvas_.setPaletteColor(IDX_PLANT_LEAF,   185, 212, 168);
  canvas_.setPaletteColor(IDX_RUG,          255, 242, 205);
  canvas_.setPaletteColor(IDX_RUG_PATTERN,  255, 188, 180);
  canvas_.setPaletteColor(IDX_WINDOW_FRAME, 218, 185, 150);
  canvas_.setPaletteColor(IDX_WINDOW_SKY,   255, 218, 200);
  canvas_.setPaletteColor(IDX_WINDOW_HILL,  208, 222, 178);
  canvas_.setPaletteColor(IDX_SHADOW,       226, 196, 162);
  canvas_.setPaletteColor(IDX_HEART,        255, 115, 125);
  canvas_.setPaletteColor(IDX_ZZZ,          235, 210, 140);
  canvas_.setPaletteColor(IDX_WINDOW_SUN,   255, 170,  90);
  canvas_.setPaletteColor(IDX_STAR,         255, 245, 200);
}

void CatPet::applyNightPalette() {
  canvas_.setPaletteColor(IDX_WALL,          60,  50,  80);
  canvas_.setPaletteColor(IDX_FLOOR,         70,  58,  75);
  canvas_.setPaletteColor(IDX_LINE,          90,  75,  95);
  canvas_.setPaletteColor(IDX_CAT_BODY,     200, 195, 185);
  canvas_.setPaletteColor(IDX_PINK,         200, 130, 140);
  canvas_.setPaletteColor(IDX_BLACK,         40,  30,  35);
  canvas_.setPaletteColor(IDX_WHITE,        220, 218, 225);
  canvas_.setPaletteColor(IDX_PLATE,        130, 140, 128);
  canvas_.setPaletteColor(IDX_FOOD,         170, 105,  70);
  canvas_.setPaletteColor(IDX_CAT_STRIPE,   180, 135,  95);
  canvas_.setPaletteColor(IDX_PLANT_POT,    140,  95,  80);
  canvas_.setPaletteColor(IDX_PLANT_LEAF,   100, 125,  90);
  canvas_.setPaletteColor(IDX_RUG,          100,  90,  75);
  canvas_.setPaletteColor(IDX_RUG_PATTERN,  130,  95,  90);
  canvas_.setPaletteColor(IDX_WINDOW_FRAME, 100,  85,  75);
  canvas_.setPaletteColor(IDX_WINDOW_SKY,    25,  20,  50);
  canvas_.setPaletteColor(IDX_WINDOW_HILL,   45,  55,  40);
  canvas_.setPaletteColor(IDX_SHADOW,        50,  42,  55);
  canvas_.setPaletteColor(IDX_HEART,        200,  85,  95);
  canvas_.setPaletteColor(IDX_ZZZ,          170, 160, 110);
  canvas_.setPaletteColor(IDX_WINDOW_SUN,   230, 225, 180);
  canvas_.setPaletteColor(IDX_STAR,         255, 255, 210);
}

void CatPet::begin() {
  canvas_.setColorDepth(8);
  canvas_.createSprite(SCR_W, SCR_H);

  applyDayPalette();

  step_ = 0;
  startStep();
}

void CatPet::startStep() {
  stepStart_ = millis();
  const ScriptStep& s = SCRIPT[step_];
  if (s.action == Action::Walk) {
    target_ = s.arg;
    dir_ = (target_ >= (int)x_) ? 1 : -1;
    walking_ = true;
    
    // Refill food plate when heading to the plate
    if (target_ == 245) {
      foodLevel_ = 3;
    }
  } else {
    walking_ = false;
    if (s.action == Action::Eat) {
      dir_ = 1;  // face the plate
    }
  }
}

void CatPet::nextStep() {
  step_ = (step_ + 1) % SCRIPT_LEN;
  startStep();
}

void CatPet::setNightMode(bool night) {
  if (night == nightMode_) return;
  nightMode_ = night;
  if (night) {
    applyNightPalette();
  } else {
    applyDayPalette();
  }
}

void CatPet::setWifiStatus(bool connected, bool timeSynced) {
  wifiConnected_ = connected;
  timeSynced_ = timeSynced;
}

void CatPet::pokeHappy() {
  mood_ = Mood::Happy;
  moodUntil_ = millis() + 2000;
}
void CatPet::pokeSleepy() {
  mood_ = Mood::Sleepy;
  moodUntil_ = millis() + 2500;
}
void CatPet::pokeAngry() {
  mood_ = Mood::Angry;
  moodUntil_ = millis() + 2000;
}

CatPet::Mood CatPet::currentMood() const {
  if (millis() < moodUntil_) return mood_;

  if (nightMode_) return Mood::Sleepy;

  if (batteryLevel_ < 20) return Mood::Sad;

  switch (SCRIPT[step_].action) {
    case Action::Eat:
    case Action::Play:
      return Mood::Happy;
    default:
      return Mood::Neutral;
  }
}

void CatPet::update() {
  frame_++;
  uint32_t now = millis();

  // Battery refresh every 5s.
  if (now - batteryStamp_ >= 5000 || batteryStamp_ == 0) {
    int lvl = M5.Power.getBatteryLevel();
    batteryLevel_ = lvl < 0 ? 0 : lvl;
    charging_ = M5.Power.isCharging();
    batteryStamp_ = now;
  }

  // Blink scheduling (suppressed while sleepy).
  if (!blinking_ && now - blinkStart_ >= BLINK_INTERVAL) {
    blinking_ = true;
    blinkStart_ = now;
  } else if (blinking_ && now - blinkStart_ >= BLINK_LEN) {
    blinking_ = false;
    blinkStart_ = now;
  }

  const ScriptStep& s = SCRIPT[step_];
  switch (s.action) {
    case Action::Walk: {
      float dx = target_ - x_;
      if (fabsf(dx) <= WALK_SPEED) {
        x_ = target_;
        walking_ = false;
        nextStep();
      } else {
        x_ += (dx > 0 ? WALK_SPEED : -WALK_SPEED);
      }
      // Bob up and down while walking to make it feel organic!
      hop_ = (frame_ % 6 < 3) ? -2.0f : 0.0f;
      break;
    }
    case Action::Play: {
      // Series of little hops.
      float t = (now - stepStart_) / 260.0f;
      hop_ = -fabsf(sinf(t * 3.14159f)) * 9.0f;
      if (now - stepStart_ >= (uint32_t)s.arg) nextStep();
      break;
    }
    case Action::Eat: {
      // Munch: gentle vertical bob toward the plate.
      hop_ = (((now - stepStart_) / 180) % 2) ? 2.0f : 0.0f;
      
      // Decrease food level during eating (4200ms duration)
      uint32_t elapsed = now - stepStart_;
      if (elapsed < 1400) foodLevel_ = 3;
      else if (elapsed < 2800) foodLevel_ = 2;
      else if (elapsed < 4000) foodLevel_ = 1;
      else foodLevel_ = 0;

      if (now - stepStart_ >= (uint32_t)s.arg) nextStep();
      break;
    }
    case Action::Sit:
    default: {
      hop_ = 0.0f;
      if (now - stepStart_ >= (uint32_t)s.arg) nextStep();
      break;
    }
  }
}

void CatPet::drawRoom() {
  canvas_.fillSprite(IDX_WALL);
  canvas_.fillRect(0, FLOOR_Y, SCR_W, SCR_H - FLOOR_Y, IDX_FLOOR);
  canvas_.fillRect(0, FLOOR_Y - 2, SCR_W, 2, IDX_LINE);
  // A few floorboard seams for depth.
  for (int fx = 24; fx < SCR_W; fx += 64) {
    canvas_.drawLine(fx, FLOOR_Y, fx - 10, SCR_H, IDX_LINE);
  }
}

void CatPet::drawWindow() {
  int wx = 90, wy = 25, ww = 140, wh = 100;
  // Draw the outer wooden frame
  canvas_.fillRect(wx, wy, ww, wh, IDX_WINDOW_FRAME);
  // Draw the inner window area
  int ix = wx + 6, iy = wy + 6, iw = ww - 12, ih = wh - 12;
  canvas_.fillRect(ix, iy, iw, ih, IDX_WINDOW_SKY);

  auto drawClippedCircle = [&](int x, int y, int r, uint8_t color) {
    for (int dy = -r; dy <= r; dy++) {
      for (int dx = -r; dx <= r; dx++) {
        if (dx*dx + dy*dy <= r*r) {
          int px = x + dx;
          int py = y + dy;
          if (px >= ix && px < ix + iw && py >= iy && py < iy + ih) {
            canvas_.drawPixel(px, py, color);
          }
        }
      }
    }
  };

  if (nightMode_) {
    // Moon (crescent: full circle minus offset circle)
    canvas_.fillCircle(ix + iw - 25, iy + 18, 10, IDX_WINDOW_SUN);
    canvas_.fillCircle(ix + iw - 20, iy + 15, 9, IDX_WINDOW_SKY);

    // Twinkling stars
    static const int starX[] = {12, 35, 55, 78, 22, 65, 95, 48};
    static const int starY[] = {10, 25, 12, 18, 42, 38, 30, 50};
    for (int i = 0; i < 8; i++) {
      int sx = ix + starX[i] % iw;
      int sy = iy + starY[i] % ih;
      if (sy < iy + ih - 18) {
        bool twinkle = ((frame_ / 8 + i * 7) % 12) < 9;
        if (twinkle) {
          canvas_.drawPixel(sx, sy, IDX_STAR);
          if (i % 3 == 0) {
            canvas_.drawPixel(sx - 1, sy, IDX_STAR);
            canvas_.drawPixel(sx + 1, sy, IDX_STAR);
            canvas_.drawPixel(sx, sy - 1, IDX_STAR);
            canvas_.drawPixel(sx, sy + 1, IDX_STAR);
          }
        }
      }
    }
  } else {
    canvas_.fillCircle(ix + 25, iy + 20, 10, IDX_WINDOW_SUN);

    int cx = ix + ((frame_ / 5) % (iw + 40)) - 25;
    if (cx + 20 >= ix && cx - 20 <= ix + iw) {
      drawClippedCircle(cx, iy + 30, 8, IDX_WHITE);
      drawClippedCircle(cx - 6, iy + 32, 6, IDX_WHITE);
      drawClippedCircle(cx + 6, iy + 32, 6, IDX_WHITE);
    }
  }

  // Rolling hills
  for (int x = ix; x < ix + iw; x++) {
    int hillY = iy + ih - 15 + (int)(sinf((x - ix) * 0.05f) * 4.0f);
    if (hillY < iy + ih) {
      canvas_.drawFastVLine(x, hillY, (iy + ih) - hillY, IDX_WINDOW_HILL);
    }
  }
  
  // Draw pane separator frame lines (wooden cross)
  canvas_.drawFastVLine(wx + ww / 2 - 2, wy + 6, wh - 12, IDX_WINDOW_FRAME);
  canvas_.drawFastVLine(wx + ww / 2 - 1, wy + 6, wh - 12, IDX_WINDOW_FRAME);
  canvas_.drawFastHLine(wx + 6, wy + wh / 2 - 2, ww - 12, IDX_WINDOW_FRAME);
  canvas_.drawFastHLine(wx + 6, wy + wh / 2 - 1, ww - 12, IDX_WINDOW_FRAME);
}

void CatPet::drawRug() {
  int rx = 160, ry = FLOOR_Y + 22, rw = 95, rh = 20;
  // Base rug oval
  canvas_.fillEllipse(rx, ry, rw, rh, IDX_RUG);
  // Inner pattern oval
  canvas_.fillEllipse(rx, ry, rw - 15, rh - 4, IDX_RUG_PATTERN);
  canvas_.fillEllipse(rx, ry, rw - 35, rh - 8, IDX_RUG);
  
  // Cute fringes on the left and right sides of the rug
  for (int dy = -rh + 3; dy <= rh - 3; dy += 3) {
    canvas_.drawFastHLine(rx - rw - 3, ry + dy, 3, IDX_RUG_PATTERN);
    canvas_.drawFastHLine(rx + rw, ry + dy, 3, IDX_RUG_PATTERN);
  }
}

void CatPet::drawPlant() {
  int px = 30, py = FLOOR_Y - 18;
  
  // Draw the pot
  canvas_.fillRect(px, py, 16, 18, IDX_PLANT_POT);
  canvas_.fillRect(px - 2, py, 20, 4, IDX_PLANT_POT); // rim
  canvas_.fillRect(px + 2, py + 18, 12, 2, IDX_SHADOW); // base shadow
  
  // Draw stems and leaves that sway
  float sway = sinf(frame_ * 0.06f) * 3.0f;
  
  // Stem 1 (center-left)
  int sx1 = px + 8;
  int sy1 = py;
  int tx1 = sx1 + (int)sway;
  int ty1 = py - 20;
  canvas_.drawLine(sx1, sy1, tx1, ty1, IDX_PLANT_LEAF);
  // Leaf clusters
  canvas_.fillCircle(tx1, ty1, 5, IDX_PLANT_LEAF);
  canvas_.fillCircle(tx1 - 5, ty1 + 5, 4, IDX_PLANT_LEAF);
  canvas_.fillCircle(tx1 + 5, ty1 + 3, 4, IDX_PLANT_LEAF);
  
  // Stem 2 (leaning left)
  int tx2 = sx1 - 10 + (int)(sway * 0.8f);
  int ty2 = py - 12;
  canvas_.drawLine(sx1, sy1, tx2, ty2, IDX_PLANT_LEAF);
  canvas_.fillCircle(tx2, ty2, 4, IDX_PLANT_LEAF);
  
  // Stem 3 (leaning right)
  int tx3 = sx1 + 10 + (int)(sway * 0.8f);
  int ty3 = py - 14;
  canvas_.drawLine(sx1, sy1, tx3, ty3, IDX_PLANT_LEAF);
  canvas_.fillCircle(tx3, ty3, 4, IDX_PLANT_LEAF);
}

void CatPet::drawGarland() {
  int gy = 12;
  // String line
  canvas_.drawLine(0, gy, 320, gy, IDX_LINE);
  
  // Draw little flag triangles along the line
  static const int flagX[] = {40, 80, 120, 160, 200, 240, 280};
  static const uint8_t flagColors[] = {IDX_PINK, IDX_RUG, IDX_PLATE, IDX_HEART, IDX_PLANT_LEAF, IDX_RUG_PATTERN, IDX_PINK};
  
  float sway = sinf(frame_ * 0.08f) * 2.0f;
  
  for (int i = 0; i < 7; i++) {
    int fx = flagX[i];
    int fy = gy;
    uint8_t color = flagColors[i];
    // Draw a triangle pointing down
    int h = 8 + (int)(sway * (i % 2 ? 1 : -1));
    canvas_.fillTriangle(fx - 5, fy, fx + 5, fy, fx, fy + h, color);
  }
}

void CatPet::drawPlate() {
  int cy = FLOOR_Y + 6;
  // Plate shadow
  canvas_.fillEllipse(PLATE_CX, cy + 2, 26, 6, IDX_SHADOW);
  // Plate itself
  canvas_.fillEllipse(PLATE_CX, cy, 26, 8, IDX_PLATE);
  canvas_.fillEllipse(PLATE_CX, cy - 1, 18, 5, IDX_LINE);
  // kibble
  if (foodLevel_ >= 1) canvas_.fillEllipse(PLATE_CX - 7, cy - 2, 4, 3, IDX_FOOD);
  if (foodLevel_ >= 2) canvas_.fillEllipse(PLATE_CX + 2, cy - 3, 4, 3, IDX_FOOD);
  if (foodLevel_ >= 3) canvas_.fillEllipse(PLATE_CX + 9, cy - 1, 3, 2, IDX_FOOD);
}

void CatPet::drawTail(int footCx, int originY) {
  int tailSide = -dir_;  // tail trails behind the direction of travel
  int baseX = footCx + tailSide * (BODY_W * PX / 2 - PX);
  int baseY = originY + 9 * PX;

  // Curve nodes: outward (nx>=0) and up (ny>=0), with radius.
  static const int NODES = 6;
  static const int NX[NODES] = {0, 5, 11, 13, 9, 2};
  static const int NY[NODES] = {0, 8, 17, 27, 35, 40};
  static const int NR[NODES] = {6, 6, 5, 5, 4, 4};

  float swing = sinf(frame_ * 0.25f) * 5.0f;

  for (int i = 0; i < NODES; i++) {
    float sway = (i >= 2) ? swing * (NY[i] / 40.0f) : 0.0f;
    int x = baseX + tailSide * (int)(NX[i] + sway);
    int y = baseY - NY[i];
    // Striped tail color!
    uint8_t tailColor = IDX_CAT_BODY;
    if (i == 2 || i == 4) {
      tailColor = IDX_CAT_STRIPE;
    }
    canvas_.fillCircle(x, y, NR[i], tailColor);
    if (i == NODES - 1) {
      canvas_.fillCircle(x, y, NR[i] - 1, IDX_WHITE);  // White tail tip
      canvas_.fillCircle(x, y, NR[i] - 2, IDX_PINK);   // Pink tip center
    }
    // Cute pink ribbon bow tied around node 3 of the tail
    if (i == 3) {
      canvas_.fillEllipse(x - 4, y - 1, 4, 3, IDX_PINK); // left bow loop
      canvas_.fillEllipse(x + 4, y - 1, 4, 3, IDX_PINK); // right bow loop
      canvas_.fillCircle(x, y, 2, IDX_WHITE); // center knot
      // ribbon tails hanging down
      canvas_.drawLine(x - 2, y + 2, x - 4, y + 6, IDX_PINK);
      canvas_.drawLine(x + 2, y + 2, x + 4, y + 6, IDX_PINK);
    }
  }
}

void CatPet::drawEye(int ex, int ey, int eyeIndex, int gaze, Mood mood,
                     bool closed) {
  // ex,ey = top-left of the 3x3 art-pixel eye box.
  if (closed || mood == Mood::Sleepy) {
    canvas_.fillRect(ex, ey + PX, 3 * PX, PX, IDX_BLACK);
    return;
  }
  if (mood == Mood::Happy) {
    // Upward arc "^_^"
    canvas_.fillRect(ex, ey + PX, PX, PX, IDX_BLACK);
    canvas_.fillRect(ex + PX, ey, PX, PX, IDX_BLACK);
    canvas_.fillRect(ex + 2 * PX, ey + PX, PX, PX, IDX_BLACK);
    return;
  }

  // Draw 3x3 eye with colored iris and black pupil
  canvas_.fillRect(ex, ey, 3 * PX, 3 * PX, IDX_WHITE);
  int col = 1 + gaze;
  if (col < 0) col = 0;
  if (col > 2) col = 2;
  
  // Draw 1x2 pupil
  canvas_.fillRect(ex + col * PX, ey + PX, PX, 2 * PX, IDX_BLACK);
  
  // Draw iris highlight below the pupil (on row 2)
  canvas_.fillRect(ex + col * PX, ey + 2 * PX, PX, PX, IDX_ZZZ); // Blue iris highlight

  bool screenLeft = (eyeIndex == 0);
  if (mood == Mood::Angry) {
    // Slant the top toward the nose.
    int mx = screenLeft ? ex + 2 * PX : ex;
    canvas_.fillRect(mx, ey, PX, PX, IDX_CAT_BODY);
  } else if (mood == Mood::Sad) {
    int mx = screenLeft ? ex : ex + 2 * PX;
    canvas_.fillRect(mx, ey, PX, PX, IDX_CAT_BODY);
  }
}

void CatPet::drawLegs(int originX, int originY, bool walking, int legPhase) {
  int top = originY + BODY_ROWS * PX;
  // Four legs at these art-pixel columns.
  static const int legCol[4] = {2, 6, 9, 13};
  for (int i = 0; i < 4; i++) {
    int lx = originX + legCol[i] * PX;
    int h = LEG_ROWS * PX;
    int y = top;
    if (walking && ((i & 1) == legPhase)) {
      h -= PX;  // lift this leg
    }
    // Main leg in IDX_CAT_BODY, paw in IDX_WHITE (socks)
    canvas_.fillRect(lx, y, 2 * PX, h - PX, IDX_CAT_BODY);
    canvas_.fillRect(lx, y + h - PX, 2 * PX, PX, IDX_WHITE);
  }
}

void CatPet::drawHeart(int x, int y, uint8_t color) {
  // 5x5 heart symbol
  canvas_.drawPixel(x + 1, y, color);
  canvas_.drawPixel(x + 3, y, color);
  canvas_.fillRect(x, y + 1, 5, 2, color);
  canvas_.fillRect(x + 1, y + 3, 3, 1, color);
  canvas_.drawPixel(x + 2, y + 4, color);
}

void CatPet::drawAngerVein(int x, int y, uint8_t color) {
  // 5x5 anger vein cross
  canvas_.drawFastVLine(x + 1, y, 5, color);
  canvas_.drawFastVLine(x + 3, y, 5, color);
  canvas_.drawFastHLine(x, y + 1, 5, color);
  canvas_.drawFastHLine(x, y + 3, 5, color);
}

void CatPet::drawSweatDrop(int x, int y, uint8_t color) {
  // 3x4 tear/sweat drop
  canvas_.drawPixel(x + 1, y, color);
  canvas_.fillRect(x, y + 1, 3, 2, color);
  canvas_.drawPixel(x + 1, y + 3, color);
}

void CatPet::drawCat() {
  int footCx = (int)x_;
  int originX = footCx - (BODY_W * PX) / 2;
  
  // Idle breathing bob:
  float breathY = 0.0f;
  if (!walking_) {
    breathY = sinf(frame_ * 0.1f) * 1.5f;
  }
  
  int originY = FLOOR_Y - CAT_PIX_H + (int)hop_ + (int)breathY;

  bool walking = walking_;
  int legPhase = (frame_ / 3) & 1;

  // Draw ground shadow under the cat
  int shadowW = 24 - (int)(-hop_ * 0.8f);
  if (shadowW < 10) shadowW = 10;
  int shadowH = 5 - (int)(-hop_ * 0.2f);
  if (shadowH < 2) shadowH = 2;
  canvas_.fillEllipse(footCx, FLOOR_Y + 1, shadowW, shadowH, IDX_SHADOW);

  // Tail behind everything.
  drawTail(footCx, originY - (int)breathY); // Tail attached to body frame, ignore breathing bob for tail base stability

  // Legs (under the body).
  drawLegs(originX, originY, walking, legPhase);

  // Body silhouette.
  for (int r = 0; r < BODY_ROWS; r++) {
    for (int c = 0; c < BODY_W; c++) {
      uint8_t v = BODY_MAP[r][c];
      if (v == 0) continue;
      uint8_t color = IDX_CAT_BODY;
      if (v == 2) color = IDX_PINK;
      else if (v == 3) color = IDX_CAT_STRIPE;
      else if (v == 4) color = IDX_WHITE;
      canvas_.fillRect(originX + c * PX, originY + r * PX, PX, PX, color);
    }
  }

  // Eyes (rows 4-6). Look toward travel; look down a touch while eating.
  Mood mood = currentMood();
  bool closed = blinking_;
  int gaze = walking ? dir_ : 0;
  if (SCRIPT[step_].action == Action::Eat && millis() >= moodUntil_) {
    closed = true;  // content, eyes shut while munching
  }
  int eyeRow = 4;
  drawEye(originX + 3 * PX, originY + eyeRow * PX, 0, gaze, mood, closed);
  drawEye(originX + 10 * PX, originY + eyeRow * PX, 1, gaze, mood, closed);

  // Floating expressions based on mood!
  if (mood == Mood::Happy) {
    for (int i = 0; i < 2; i++) {
      uint32_t phase = (frame_ + i * 60) % 120;
      float t = phase / 120.0f;
      int hx = footCx + (int)(cosf(t * 5.0f) * 20.0f) - 2;
      int hy = originY - 15 - (int)(t * 45.0f);
      drawHeart(hx, hy, IDX_HEART);
    }
  } else if (mood == Mood::Sleepy) {
    for (int i = 0; i < 3; i++) {
      uint32_t phase = (frame_ + i * 40) % 120;
      float t = phase / 120.0f;
      int zx = footCx + (int)(sinf(t * 6.28f) * 15.0f) + 12;
      int zy = originY - 10 - (int)(t * 40.0f);
      int size = (t < 0.2f || t > 0.8f) ? 1 : 2;
      if (size == 1) {
        canvas_.drawFastHLine(zx, zy, 3, IDX_ZZZ);
        canvas_.drawPixel(zx + 1, zy + 1, IDX_ZZZ);
        canvas_.drawFastHLine(zx, zy + 2, 3, IDX_ZZZ);
      } else {
        canvas_.drawFastHLine(zx, zy, 5, IDX_ZZZ);
        canvas_.drawLine(zx + 4, zy, zx, zy + 4, IDX_ZZZ);
        canvas_.drawFastHLine(zx, zy + 4, 5, IDX_ZZZ);
      }
    }
  } else if (mood == Mood::Angry) {
    int ax = footCx + 15 + (frame_ % 4 >= 2 ? 1 : -1);
    int ay = originY - 5 + (frame_ % 2 ? 1 : -1);
    drawAngerVein(ax, ay, IDX_HEART);
  } else if (mood == Mood::Sad) {
    uint32_t phase = frame_ % 80;
    float t = phase / 80.0f;
    int sx = footCx - 18;
    int sy = originY + 10 + (int)(t * 15.0f);
    if (t < 0.9f) {
      drawSweatDrop(sx, sy, IDX_ZZZ);
    }
  }
}

void CatPet::drawBattery() {
  int x = 278, y = 12, w = 30, h = 14;
  canvas_.drawRoundRect(x, y, w, h, 3, IDX_LINE);
  canvas_.fillRect(x + w, y + 4, 2, h - 8, IDX_LINE);  // nub
  
  uint8_t fillCol = IDX_PLATE; // good (mint)
  if (batteryLevel_ < 25) {
    fillCol = IDX_HEART; // low (cherry pink)
  } else if (batteryLevel_ < 60) {
    fillCol = IDX_FOOD; // medium (pastel orange)
  }
  
  int maxFillW = w - 6;
  int fillw = (int)(maxFillW * (batteryLevel_ / 100.0f));
  if (fillw > 0) {
    canvas_.fillRect(x + 3, y + 3, fillw, h - 6, fillCol);
  }
  
  if (charging_) {
    if ((frame_ / 10) % 2 == 0) {
      int cx = x + w / 2;
      int cy = y + h / 2;
      canvas_.drawPixel(cx, cy - 2, IDX_WHITE);
      canvas_.drawPixel(cx - 1, cy - 1, IDX_WHITE);
      canvas_.drawPixel(cx, cy - 1, IDX_WHITE);
      canvas_.drawPixel(cx, cy, IDX_WHITE);
      canvas_.drawPixel(cx + 1, cy, IDX_WHITE);
      canvas_.drawPixel(cx, cy + 1, IDX_WHITE);
      canvas_.drawPixel(cx - 1, cy + 2, IDX_WHITE);
    }
  }
}

void CatPet::drawWifiIcon() {
  int x = 255, y = 14;
  uint8_t col = wifiConnected_ ? IDX_PLATE : IDX_HEART;
  if (!wifiConnected_ && (frame_ / 15) % 2 == 0) return;

  // Three arcs (small WiFi symbol)
  for (int r = 3; r <= 9; r += 3) {
    for (int a = -r; a <= r; a++) {
      int dy = -(int)sqrtf((float)(r * r - a * a));
      int px = x + a;
      int py = y + 10 + dy;
      if (a * a + dy * dy >= (r - 1) * (r - 1)) {
        canvas_.drawPixel(px, py, col);
      }
    }
  }
  canvas_.fillRect(x - 1, y + 9, 3, 3, col);
}

void CatPet::render() {
  drawRoom();
  drawWindow();
  drawRug();
  drawPlant();
  drawGarland();
  drawPlate();
  drawCat();
  drawBattery();
  drawWifiIcon();
  canvas_.pushSprite(0, 0);
}
