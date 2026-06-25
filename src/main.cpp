#include <M5Unified.h>
#include <Avatar.h>

using namespace m5avatar;

Avatar avatar;

void setup() {
    auto cfg = M5.config();
    M5.begin(cfg);
    M5.Display.setBrightness(128);

    avatar.init();
    avatar.setColorPalette(ColorPalette());
}

void loop() {
    // Button A: happy expression
    M5.update();

    if (M5.BtnA.wasPressed()) {
        avatar.setExpression(Expression::Happy);
        delay(2000);
        avatar.setExpression(Expression::Neutral);
    }

    if (M5.BtnB.wasPressed()) {
        avatar.setExpression(Expression::Sleepy);
        delay(2000);
        avatar.setExpression(Expression::Neutral);
    }

    if (M5.BtnC.wasPressed()) {
        avatar.setExpression(Expression::Angry);
        delay(2000);
        avatar.setExpression(Expression::Neutral);
    }
}
