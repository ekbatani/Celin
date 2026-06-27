#include <M5Unified.h>

#include "AiClient.h"
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
static const uint8_t BRIGHTNESS_STORY = 80;

// --- AI text pager ---
enum class AiPhase { Idle, StoryIntro, Paging, StoryEnd };
static AiPhase aiPhase = AiPhase::Idle;
static uint32_t aiTimer = 0;
static bool storyToldTonight = false;
static bool aiIsStory = false;

static const int PAGE_LEN = 42;
static const uint32_t PAGE_MS = 5000;
static char pageBuf[50] = {};
static int pageOffset = 0;

static int nextPageBreak(const char* text, int maxLen) {
    int total = strlen(text);
    if (total <= maxLen) return total;
    int i = maxLen;
    while (i > 0 && text[i] != ' ') i--;
    return i > 0 ? i : maxLen;
}

static bool showNextPage() {
    const char* text = AiClient::getResponse();
    int remaining = strlen(text) - pageOffset;
    if (remaining <= 0) return false;
    int len = nextPageBreak(text + pageOffset, PAGE_LEN);
    strncpy(pageBuf, text + pageOffset, len);
    pageBuf[len] = '\0';
    pageOffset += len;
    if (text[pageOffset] == ' ') pageOffset++;
    pet.showBubble(pageBuf, PAGE_MS);
    aiTimer = millis();
    return true;
}

static void startStory() {
    if (aiPhase != AiPhase::Idle) return;
    aiPhase = AiPhase::StoryIntro;
    aiTimer = millis();
    aiIsStory = true;
    pet.setStoryMode(true);
    pet.showBubble("Story time!", 2500);
    Sound::storyChime();
}

static void forceRender() {
    pet.update();
    pet.render();
    lastFrame = millis();
}

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setBrightness(BRIGHTNESS_DAY);

    pet.begin();
    Net::begin();
    Sound::begin();
    Weather::begin();
    Messaging::begin();
    AiClient::begin();
}

void loop() {
    M5.update();

    // --- Messaging + AI chat ---
    Messaging::update();
    if (Messaging::hasNewMessage()) {
        const char* msg = Messaging::getLastMessage();

        if (strncmp(msg, "/ask ", 5) == 0 && aiPhase == AiPhase::Idle) {
            if (pet.isManualSleeping()) pet.wakeUp();
            pet.showBubble("Hmm...", 30000);
            forceRender();

            if (AiClient::ask(msg + 5)) {
                pageOffset = 0;
                aiPhase = AiPhase::Paging;
                aiIsStory = false;
                showNextPage();
                char reply[550];
                snprintf(reply, sizeof(reply), "\xf0\x9f\x90\xb1 %s",
                         AiClient::getResponse());
                Messaging::sendMessage(reply);
            } else {
                pet.showBubble("Can't think now", 3000);
            }
        } else if (strcmp(msg, "/story") == 0) {
            if (pet.isManualSleeping()) pet.wakeUp();
            startStory();
        } else {
            if (pet.isManualSleeping()) pet.wakeUp();
            pet.showBubble(msg, 5000);
            Sound::notification();
        }
        Messaging::clearMessage();
    }

    // --- Buttons (disabled during AI phases) ---
    if (aiPhase == AiPhase::Idle) {
        if (pet.isManualSleeping()) {
            if (M5.BtnA.wasPressed() || M5.BtnB.wasPressed() ||
                M5.BtnC.wasPressed()) {
                pet.wakeUp();
            }
        } else if (!pet.isNightMode()) {
            if (M5.BtnA.wasHold()) {
                Messaging::sendMessage("\xf0\x9f\x90\xb1 Meow! I love you!");
                pet.showBubble("Love you!", 3000);
                Sound::happyMeow();
            } else if (M5.BtnB.wasHold()) {
                Messaging::sendMessage(
                    "\xf0\x9f\x90\xb1 Come play with me!");
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
    }

    // --- Background updates ---
    Net::update();
    Sound::update();
    Weather::update();

    pet.setWifiStatus(Net::isConnected(), Net::isTimeSynced());

    if (Weather::hasData()) {
        pet.setWeather(Weather::temperature(), Weather::conditionId());
    }

    // --- Day/night cycle ---
    if (Net::isTimeSynced()) {
        int h = Net::hour();
        bool night = (h >= NIGHT_START_HOUR || h < NIGHT_END_HOUR);
        if (night != pet.isNightMode()) {
            pet.setNightMode(night);
            if (night) {
                if (!storyToldTonight) {
                    M5.Display.setBrightness(BRIGHTNESS_STORY);
                    startStory();
                } else {
                    M5.Display.setBrightness(BRIGHTNESS_NIGHT);
                }
            } else {
                M5.Display.setBrightness(BRIGHTNESS_DAY);
                storyToldTonight = false;
                if (pet.isManualSleeping()) pet.wakeUp();
            }
        }
    }

    // --- AI story state machine ---
    if (aiPhase == AiPhase::StoryIntro && millis() - aiTimer >= 2500) {
        pet.showBubble("Hmm...", 30000);
        forceRender();

        if (AiClient::fetchStory()) {
            char reply[550];
            snprintf(reply, sizeof(reply), "\xf0\x9f\x93\x96 %s",
                     AiClient::getResponse());
            Messaging::sendMessage(reply);

            pageOffset = 0;
            aiPhase = AiPhase::Paging;
            showNextPage();
        } else {
            pet.showBubble("*yawn*", 2000);
            aiPhase = AiPhase::StoryEnd;
            aiTimer = millis();
        }
    }

    if (aiPhase == AiPhase::Paging && millis() - aiTimer >= PAGE_MS) {
        if (!showNextPage()) {
            if (aiIsStory) {
                aiPhase = AiPhase::StoryEnd;
                aiTimer = millis();
                pet.showBubble("The end!", 3000);
                Sound::sleepyMeow();
            } else {
                aiPhase = AiPhase::Idle;
            }
        }
    }

    if (aiPhase == AiPhase::StoryEnd && millis() - aiTimer >= 3000) {
        aiPhase = AiPhase::Idle;
        storyToldTonight = true;
        pet.setStoryMode(false);
        M5.Display.setBrightness(BRIGHTNESS_NIGHT);
        pet.triggerSleep();
    }

    // --- Frame update ---
    unsigned long now = millis();
    if (now - lastFrame >= FRAME_MS) {
        lastFrame = now;
        pet.update();
        pet.render();
    }
}
