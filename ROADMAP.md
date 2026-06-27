# 🐱 Cat Avatar — Project Roadmap

A fun, safe, AI-powered cat companion for kids, running on an M5Stack Core (ESP32).

---

## Current State

- PlatformIO project using `M5Unified`.
- A tiny **pixel-art lilac cat** (Tamagotchi / codachi style) that walks around a cozy
  room — floor + food plate — driven by a looping action state machine, with a
  swishing tail and button-driven moods (Happy / Sleepy / Angry). See
  [`src/CatPet.cpp`](src/CatPet.cpp). Rendered to a full-screen back-buffer for
  flicker-free animation; main loop is non-blocking (`millis()`).
- **WiFi + NTP time sync** with auto-reconnect, connection-status icon, and a full day/night cycle: at night (20:00–07:00) the palette shifts to moonlit blues/purples, the window shows a crescent moon and twinkling stars, the cat goes sleepy, and the display dims. See [`src/Net.cpp`](src/Net.cpp).
- No sound, no AI — these will be added in later phases.

### Hardware
- **Board:** M5Stack Core V2.7 (ESP32-D0WDQ6-V3) — port `/dev/ttyACM0` @ 115200.
- **Input:** 3 physical buttons (A / B / C).
- **Output:** 320×240 display, built-in speaker (low quality).
- **⚠️ Limitation:** The base Core has **no built-in microphone**. Voice input requires an external microphone (e.g. an I2S MEMS mic like the SPM1423/INMP441); otherwise AI input is done via text (Telegram).

---

## Phases

The roadmap is split into 5 phases. Each phase builds on the previous one. The "Depends on" column shows what a feature requires.

---

### Phase 0 — Foundation & Character

| # | Goal | Technical notes | Depends on |
|---|------|-----------------|------------|
| ✅ | Initial button-driven expressions | Already in place | — |
| ✅ | **Playful cat with white ears** | Custom face (`Face` / `Drawable`) in M5Stack-Avatar; add cat ears + white coloring. Remove blocking `delay()` calls from `loop()`. | — |
| ✅ | **Battery level display** | `M5.Power.getBatteryLevel()` shown as an icon/percentage in a screen corner, refreshed every few seconds. | — |

**Phase output:** A cute cat with personality that shows its charge and has a non-blocking main loop.

---

### Phase 1 — Connectivity & Time

| # | Goal | Technical notes | Depends on |
|---|------|-----------------|------------|
| ✅ | **Connect to WiFi** | `WiFi.begin()`; SSID/password in a separate file (`include/secrets.h`, outside the repo). Offline/reconnect handling. Connection-status icon. | — |
| ✅ | **Day & night** | Sync time over NTP (`configTime`). Change the background/color palette between day (light) and night (dark) based on the hour. | WiFi |
| ✅ | **Sleep at night** | After a set hour, the avatar goes into `Sleepy` mode and the display dims; it wakes up in the morning. | Day & night |

**Phase output:** The cat knows whether it's day or night and adjusts its behavior.

---

### Phase 2 — Interaction & Play

| # | Goal | Technical notes | Depends on |
|---|------|-----------------|------------|
| ☐ | **Feed button** | On button press, play a "go to food and eat" animation + speech bubble + short sound. Internal "hunger" state. | Phase 0 |
| ☐ | **Play button** | Playful animation (jumping/chasing), Happy expression, sound. | Phase 0 |
| ☐ | **Go to sleep (manual)** | A button/command to put the avatar to sleep. | Phase 0 |
| ☐ | **Sound** | Set up the built-in speaker (`M5.Speaker`) for simple sound effects (meow, notification). | — |

**Phase output:** A kid can use the buttons to feed the cat, play with it, and put it to sleep.

---

### Phase 3 — Weather & The World

| # | Goal | Technical notes | Depends on |
|---|------|-----------------|------------|
| ☐ | **Show weather every few hours** | Call a weather API (free OpenWeatherMap) every N hours; display temperature/icon. Non-blocking scheduling with `millis()`. | WiFi |
| ☐ | **React to hot/cold** | If temperature > hot threshold → overheated/sweaty expression; if < cold threshold → shivering expression. Configurable thresholds. | Weather |

**Phase output:** The cat reacts to your city's real weather.

---

### Phase 4 — Messaging

| # | Goal | Technical notes | Depends on |
|---|------|-----------------|------------|
| ☐ | **Message mom & dad** | Telegram bot (the `UniversalTelegramBot` library). On an event (e.g. a button press or a specific occurrence) it sends a message to the parents' chat. | WiFi |
| ☐ | **Receive messages from us** | The same Telegram bot reads incoming messages and shows them as a speech bubble on screen + a notification sound. | WiFi, Sound |

**Phase output:** Two-way communication between the cat and the parents via Telegram.

> **Why Telegram?** On the ESP32 it's simple, free, two-way, and secure (HTTPS), with no server required. Alternatives: email (SMTP) or push notifications.

---

### Phase 5 — AI Assistant

| # | Goal | Technical notes | Depends on |
|---|------|-----------------|------------|
| ☐ | **Tell a story at night** | At night before sleep, fetch a short story from the Claude API (or pick from a locally stored list for offline mode) and narrate it on screen/with sound. | WiFi, Day & night |
| ☐ | **AI assistant that answers our questions** | Connect to the **Claude API** (a low-cost model like Haiku for fast replies). Input: text from Telegram (or voice, if a mic is added). Output: text on screen + sound. | WiFi, Telegram |
| ☐ | **Fun, kid-safe personality** | A strict system prompt: kind and child-friendly tone, inappropriate-content filtering, short and simple answers. Topic restrictions. | AI assistant |

**Phase output:** The cat becomes a smart friend that tells stories and safely answers kids' questions.

> **AI note:** Calling the API directly from the ESP32 is possible (HTTPS), but keeping the API key on the device is a security risk. The safer option: a lightweight proxy (e.g. a small Worker/server) that holds the key, enforces the safety system prompt, and is the only thing the ESP32 talks to.

---

## Suggested Code Architecture

To keep `main.cpp` from getting crowded, split the code into modules:

```
src/
  main.cpp            ← setup + non-blocking frame loop + buttons
  CatPet.{h,cpp}      ← pixel-art cat sprite, room/food scene, action state machine
  Net.{h,cpp}         ← WiFi + NTP + time sync
  Weather.{h,cpp}     ← fetch and cache weather
  Messaging.{h,cpp}   ← Telegram bot (send/receive)
  AiClient.{h,cpp}    ← Claude API calls (stories + Q&A)
  Sound.{h,cpp}       ← sound effects
  AppState.h          ← states (hunger, asleep/awake, day/night)
include/
  secrets.h           ← SSID, password, tokens (in .gitignore)
```

**State machine:** The avatar is always in one of these states — `IDLE`, `EATING`, `PLAYING`, `SLEEPING`, `STORYTIME`, `CHATTING`, `WEATHER_REACT`. The main loop must be **non-blocking** (use `millis()` instead of `delay()`) so WiFi, buttons, and animation all run concurrently.

---

## Recommended Build Order

1. **Phase 0** — Cat character + battery + remove delays (the basis for everything).
2. **Phase 1** — WiFi + time + day/night cycle.
3. **Phase 2** — Feed/play/sleep buttons + sound (fun and tangible for a kid).
4. **Phase 3** — Weather and reactions.
5. **Phase 4** — Telegram messaging.
6. **Phase 5** — Stories and AI assistant (the most complex; last).

---

## Full Goal Checklist

- [x] Have a day and night cycle — *Phase 1*
- [x] Be a playful cat with white ears — *Phase 0*
- [ ] Show the weather every few hours — *Phase 3*
- [x] Connect to WiFi — *Phase 1*
- [ ] Press a button to put out food and have it go eat — *Phase 2*
- [ ] A button to play — *Phase 2*
- [ ] Go to sleep — *Phase 2*
- [ ] Message mom and dad — *Phase 4*
- [ ] React if the weather is hot or cold — *Phase 3*
- [x] Sleep at night — *Phase 1*
- [ ] Tell a story at night, then sleep — *Phase 5*
- [x] Show the battery level — *Phase 0*
- [ ] Receive messages from us — *Phase 4*
- [ ] Become a fun, safe AI personal assistant for kids — *Phase 5*
- [ ] Connect to AI and answer our questions — *Phase 5*

---

## Libraries to Add

To be added to `platformio.ini`:

```ini
lib_deps =
    m5stack/M5Unified@^0.2.2
    meganetaaan/M5Stack-Avatar@^0.10.0
    bblanchon/ArduinoJson              ; parse weather/AI API responses
    witnessmenow/UniversalTelegramBot  ; Telegram bot
    ; WiFi, HTTPClient, WiFiClientSecure come from the ESP32 framework itself
```
