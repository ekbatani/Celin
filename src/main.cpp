#include <M5Unified.h>
#include "CatPet.h"
#include "Messaging.h"
#include "Net.h"
#include "Sound.h"
#include "Weather.h"

CatPet pet;

const unsigned long FRAME_MS = 40;  // ~25 fps
unsigned long lastFrame = 0;

static const int NIGHT_START_HOUR = 20;
static const int NIGHT_END_HOUR = 7;

static const uint8_t BRIGHTNESS_DAY = 128;
static const uint8_t BRIGHTNESS_NIGHT = 30;

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setBrightness(BRIGHTNESS_DAY);

    pet.begin();
    Net::begin();
    Sound::begin();
    Weather::begin();
    Messaging::begin();
}

void loop() {
    M5.update();

    Messaging::update();
    if (Messaging::hasNewMessage()) {
        if (pet.isManualSleeping()) pet.wakeUp();
        pet.showBubble(Messaging::getLastMessage(), 5000);
        Sound::notification();
        Messaging::clearMessage();
    }

    if (pet.isManualSleeping()) {
        if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() || M5.BtnC.wasPressed()) {
            pet.wakeUp();
        }
    } else if (!pet.isNightMode()) {
        if (M5.BtnA.wasHold()) {
            Messaging::sendMessage("\xf0\x9f\x90\xb1 Meow! I love you!");
            pet.showBubble("Love you!", 3000);
            Sound::happyMeow();
        } else if (M5.BtnB.wasHold()) {
            Messaging::sendMessage("\xf0\x9f\x90\xb1 Come play with me!");
            pet.showBubble("Play time!", 3000);
            Sound::happyMeow();
        } else if (M5.BtnA.wasClicked()) {
            pet.triggerFeed();
        } else if (M5.BtnB.wasClicked()) {
            pet.triggerPlay();
        } else if (M5.BtnC.wasClicked()) {
            pet.triggerSleep();
        }
    }

    Net::update();
    Sound::update();
    Weather::update();

    pet.setWifiStatus(Net::isConnected(), Net::isTimeSynced());

    if (Weather::hasData()) {
        pet.setWeather(Weather::temperature(), Weather::conditionId());
    }

    if (Net::isTimeSynced()) {
        int h = Net::hour();
        bool night = (h >= NIGHT_START_HOUR || h < NIGHT_END_HOUR);
        if (night != pet.isNightMode()) {
            pet.setNightMode(night);
            M5.Display.setBrightness(night ? BRIGHTNESS_NIGHT : BRIGHTNESS_DAY);
            if (!night && pet.isManualSleeping()) {
                pet.wakeUp();
            }
        }
    }

    unsigned long now = millis();
    if (now - lastFrame >= FRAME_MS) {
        lastFrame = now;
        pet.update();
        pet.render();
    }
}
