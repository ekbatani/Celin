#ifndef CAT_PET_H_
#define CAT_PET_H_

#include <M5Unified.h>

class CatPet {
 public:
  enum class Action { Walk, Sit, Eat, Play };
  enum class Mood { Neutral, Happy, Sleepy, Angry, Sad };
  enum class Interactive {
    None,
    FeedWalk,
    FeedEat,
    FeedReturn,
    PlayBounce,
    SleepWalk,
    SleepRest
  };

  void begin();
  void update();
  void render();

  void setNightMode(bool night);
  bool isNightMode() const { return nightMode_; }

  void setWifiStatus(bool connected, bool timeSynced);

  void triggerFeed();
  void triggerPlay();
  void triggerSleep();
  void wakeUp();
  bool isManualSleeping() const { return manualSleep_; }

  void setWeather(float temp, int conditionId);

 private:
  M5Canvas canvas_{&M5.Display};

  // --- motion / position ---
  float x_ = 60.0f;
  int dir_ = 1;
  bool walking_ = false;
  float hop_ = 0.0f;

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

  // --- hunger ---
  int hunger_ = 100;
  int foodLevel_ = 3;
  uint32_t lastHungerTick_ = 0;

  // --- day/night ---
  bool nightMode_ = false;

  // --- connectivity ---
  bool wifiConnected_ = false;
  bool timeSynced_ = false;

  // --- interactive actions ---
  Interactive interactive_ = Interactive::None;
  uint32_t interactiveStart_ = 0;
  float returnX_ = 60.0f;
  bool manualSleep_ = false;

  // --- speech bubble ---
  const char* bubbleText_ = nullptr;
  uint32_t bubbleUntil_ = 0;

  // --- weather ---
  float weatherTemp_ = 0;
  int weatherCondition_ = 0;
  bool hasWeather_ = false;

  void nextStep();
  void startStep();
  Mood currentMood() const;
  void setBubble(const char* text, uint32_t durationMs);

  void drawRoom();
  void drawWindow();
  void drawRug();
  void drawPlant();
  void drawGarland();
  void drawPlate();
  void drawCat();
  void drawTail(int footCx, int originY);
  void drawEye(int ex, int ey, int eyeIndex, int gaze, Mood mood, bool closed);
  void drawLegs(int originX, int originY, bool walking, int legPhase);
  void drawBattery();
  void drawWifiIcon();
  void drawHeart(int x, int y, uint8_t color);
  void drawAngerVein(int x, int y, uint8_t color);
  void drawSweatDrop(int x, int y, uint8_t color);
  void drawSpeechBubble();
  void drawWeather();
  void drawButtonLabels();

  void applyDayPalette();
  void applyNightPalette();
};

#endif  // CAT_PET_H_
