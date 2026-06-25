#include <M5Unified.h>
#include "CatPet.h"

CatPet pet;

const unsigned long FRAME_MS = 40;  // ~25 fps
unsigned long lastFrame = 0;

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setBrightness(128);

    pet.begin();
}

void loop() {
    M5.update();

    if (M5.BtnA.wasPressed()) pet.pokeHappy();
    if (M5.BtnB.wasPressed()) pet.pokeSleepy();
    if (M5.BtnC.wasPressed()) pet.pokeAngry();

    unsigned long now = millis();
    if (now - lastFrame >= FRAME_MS) {
        lastFrame = now;
        pet.update();
        pet.render();
    }
}
