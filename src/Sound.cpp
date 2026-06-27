#include "Sound.h"

#include <M5Unified.h>

namespace {

struct Note {
  uint16_t freq;
  uint16_t durationMs;
  uint16_t pauseMs;
};

const int MAX_NOTES = 8;
Note queue[MAX_NOTES];
int queueLen = 0;
int queuePos = 0;
uint32_t noteStart = 0;

void play(const Note* notes, int count) {
  int n = count < MAX_NOTES ? count : MAX_NOTES;
  for (int i = 0; i < n; i++) queue[i] = notes[i];
  queueLen = n;
  queuePos = 0;
  noteStart = millis();
  M5.Speaker.tone(queue[0].freq, queue[0].durationMs);
}

}  // namespace

void Sound::begin() {
  M5.Speaker.setVolume(64);
}

void Sound::update() {
  if (queuePos >= queueLen) return;
  uint32_t now = millis();
  const Note& n = queue[queuePos];
  if (now - noteStart >= n.durationMs + n.pauseMs) {
    queuePos++;
    if (queuePos < queueLen) {
      noteStart = now;
      M5.Speaker.tone(queue[queuePos].freq, queue[queuePos].durationMs);
    }
  }
}

void Sound::meow() {
  static const Note notes[] = {{800, 80, 30}, {600, 150, 0}};
  play(notes, 2);
}

void Sound::happyMeow() {
  static const Note notes[] = {{900, 60, 20}, {1100, 60, 20}, {1300, 120, 0}};
  play(notes, 3);
}

void Sound::munch() {
  static const Note notes[] = {
      {200, 50, 60}, {300, 50, 60}, {200, 50, 60}, {300, 50, 0}};
  play(notes, 4);
}

void Sound::sleepyMeow() {
  static const Note notes[] = {{500, 180, 60}, {350, 250, 0}};
  play(notes, 2);
}

void Sound::playJingle() {
  static const Note notes[] = {
      {523, 80, 30}, {659, 80, 30}, {784, 100, 30}, {1047, 150, 0}};
  play(notes, 4);
}

void Sound::notification() {
  static const Note notes[] = {{1200, 80, 40}, {1600, 120, 0}};
  play(notes, 2);
}
