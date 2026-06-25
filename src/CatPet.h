#ifndef CAT_PET_H_
#define CAT_PET_H_

#include <M5Unified.h>

// A tiny pixel-art cat that walks around a cozy room (Tamagotchi / codachi
// style). Owns a full-screen back-buffer and renders the whole scene each
// frame: wall, floor, food plate, the cat (with a swishing tail), and a
// battery indicator. A looping action state machine drives the behaviour.
class CatPet {
 public:
  enum class Action { Walk, Sit, Eat, Play };
  enum class Mood { Neutral, Happy, Sleepy, Angry, Sad };

  void begin();
  // Advance the simulation. Call once per animation frame.
  void update();
  // Draw the scene to the back-buffer and push it to the display.
  void render();

  // Button hooks trigger short playful reactions.
  void pokeHappy();
  void pokeSleepy();
  void pokeAngry();

 private:

  M5Canvas canvas_{&M5.Display};

  // --- motion / position ---
  float x_ = 60.0f;       // foot-center x on the floor
  int dir_ = 1;           // facing / travel direction: +1 right, -1 left
  bool walking_ = false;
  float hop_ = 0.0f;      // vertical hop offset (Play / Happy)

  // --- animation clock ---
  uint32_t frame_ = 0;

  // --- action script ---
  int step_ = 0;
  uint32_t stepStart_ = 0;
  int target_ = 60;

  // --- blink + mood ---
  uint32_t blinkStart_ = 0;
  bool blinking_ = false;
  Mood mood_ = Mood::Neutral;
  uint32_t moodUntil_ = 0;

  // --- battery ---
  int batteryLevel_ = 100;
  bool charging_ = false;
  uint32_t batteryStamp_ = 0;

  void nextStep();
  void startStep();
  Mood currentMood() const;

  // drawing helpers (operate on canvas_)
  void drawRoom();
  void drawPlate();
  void drawCat();
  void drawTail(int footCx, int originY);
  void drawEye(int ex, int ey, int eyeIndex, int gaze, Mood mood, bool closed);
  void drawLegs(int originX, int originY, bool walking, int legPhase);
  void drawBattery();
};

#endif  // CAT_PET_H_
