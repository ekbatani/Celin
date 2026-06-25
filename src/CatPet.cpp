#include "CatPet.h"

#include <math.h>

// ---- palette indices (8-bit sprite) ----
static const uint8_t IDX_WALL = 0;
static const uint8_t IDX_FLOOR = 1;
static const uint8_t IDX_LINE = 2;
static const uint8_t IDX_CAT = 3;
static const uint8_t IDX_PINK = 4;
static const uint8_t IDX_BLACK = 5;
static const uint8_t IDX_WHITE = 6;
static const uint8_t IDX_PLATE = 7;
static const uint8_t IDX_FOOD = 8;

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

// Lilac silhouette: 0 empty, 1 lilac body, 2 pink (ears/nose/blush).
static const uint8_t BODY_MAP[BODY_ROWS][BODY_W] = {
    {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0},
    {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0},
    {0, 1, 2, 1, 0, 0, 0, 0, 0, 0, 0, 1, 2, 1, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1},
    {1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
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

void CatPet::begin() {
  canvas_.setColorDepth(8);
  canvas_.createSprite(SCR_W, SCR_H);

  canvas_.setPaletteColor(IDX_WALL, 26, 22, 40);
  canvas_.setPaletteColor(IDX_FLOOR, 58, 46, 72);
  canvas_.setPaletteColor(IDX_LINE, 96, 76, 116);
  canvas_.setPaletteColor(IDX_CAT, 202, 172, 212);
  canvas_.setPaletteColor(IDX_PINK, 246, 162, 192);
  canvas_.setPaletteColor(IDX_BLACK, 18, 14, 26);
  canvas_.setPaletteColor(IDX_WHITE, 248, 246, 255);
  canvas_.setPaletteColor(IDX_PLATE, 206, 206, 218);
  canvas_.setPaletteColor(IDX_FOOD, 242, 142, 64);

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
  // Idle moods derived from the current action.
  switch (SCRIPT[step_].action) {
    case Action::Eat:
      return Mood::Happy;
    case Action::Play:
      return Mood::Happy;
    case Action::Sit:
      return Mood::Neutral;
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
      hop_ = 0.0f;
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

void CatPet::drawPlate() {
  int cy = FLOOR_Y + 6;
  canvas_.fillEllipse(PLATE_CX, cy, 26, 8, IDX_PLATE);
  canvas_.fillEllipse(PLATE_CX, cy - 1, 18, 5, IDX_LINE);
  // kibble
  canvas_.fillEllipse(PLATE_CX - 7, cy - 2, 4, 3, IDX_FOOD);
  canvas_.fillEllipse(PLATE_CX + 2, cy - 3, 4, 3, IDX_FOOD);
  canvas_.fillEllipse(PLATE_CX + 9, cy - 1, 3, 2, IDX_FOOD);
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
    canvas_.fillCircle(x, y, NR[i], IDX_CAT);
    if (i == NODES - 1) {
      canvas_.fillCircle(x, y, NR[i] - 2, IDX_PINK);  // soft tail tip
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

  canvas_.fillRect(ex, ey, 3 * PX, 3 * PX, IDX_WHITE);
  int col = 1 + gaze;
  if (col < 0) col = 0;
  if (col > 2) col = 2;
  canvas_.fillRect(ex + col * PX, ey + PX, PX, PX, IDX_BLACK);

  bool screenLeft = (eyeIndex == 0);
  if (mood == Mood::Angry) {
    // Slant the top toward the nose.
    int mx = screenLeft ? ex + 2 * PX : ex;
    canvas_.fillRect(mx, ey, PX, PX, IDX_CAT);
  } else if (mood == Mood::Sad) {
    int mx = screenLeft ? ex : ex + 2 * PX;
    canvas_.fillRect(mx, ey, PX, PX, IDX_CAT);
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
    canvas_.fillRect(lx, y, 2 * PX, h, IDX_CAT);
    canvas_.fillRect(lx, y + h - PX, 2 * PX, PX, IDX_PINK);  // paw pad
  }
}

void CatPet::drawCat() {
  int footCx = (int)x_;
  int originX = footCx - (BODY_W * PX) / 2;
  int originY = FLOOR_Y - CAT_PIX_H + (int)hop_;

  bool walking = walking_;
  int legPhase = (frame_ / 3) & 1;

  // Tail behind everything.
  drawTail(footCx, originY);

  // Legs (under the body).
  drawLegs(originX, originY, walking, legPhase);

  // Body silhouette.
  for (int r = 0; r < BODY_ROWS; r++) {
    for (int c = 0; c < BODY_W; c++) {
      uint8_t v = BODY_MAP[r][c];
      if (v == 0) continue;
      uint8_t color = (v == 1) ? IDX_CAT : IDX_PINK;
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
}

void CatPet::drawBattery() {
  int x = 286, y = 8, w = 26, h = 12;
  canvas_.drawRect(x, y, w, h, IDX_WHITE);
  canvas_.fillRect(x + w, y + 3, 2, h - 6, IDX_WHITE);  // nub
  int fillw = (int)((w - 4) * (batteryLevel_ / 100.0f));
  if (fillw > 0) {
    uint8_t c = charging_ ? IDX_FOOD : IDX_CAT;
    canvas_.fillRect(x + 2, y + 2, fillw, h - 4, c);
  }
}

void CatPet::render() {
  drawRoom();
  drawPlate();
  drawCat();
  drawBattery();
  canvas_.pushSprite(0, 0);
}
