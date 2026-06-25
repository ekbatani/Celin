# 🐱 Celin

A fun, safe, AI-powered cat companion for kids — running on an **M5Stack Core (ESP32)**.

Celin is a little on-screen cat with personality: it knows day from night, reacts to the
weather, can be fed and played with, tells bedtime stories, and (eventually) answers kids'
questions through a kid-safe AI assistant.

> See [ROADMAP.md](ROADMAP.md) for the full feature plan and build order.

---

## Hardware

- **Board:** M5Stack Core V2.7 (ESP32-D0WDQ6-V3)
- **Display:** 320×240
- **Input:** 3 physical buttons (A / B / C)
- **Output:** built-in speaker
- **Serial port:** `/dev/ttyACM0` @ 115200 baud

> ⚠️ The base Core has **no built-in microphone**. Voice input requires an external I2S MEMS
> mic (e.g. SPM1423 / INMP441); otherwise AI input is done via text.

---

## Tech Stack

- **Toolchain:** [PlatformIO](https://platformio.org/) (Arduino framework on `espressif32`)
- **Libraries:**
  - [`M5Unified`](https://github.com/m5stack/M5Unified) — board/display/button/power abstraction
  - [`M5Stack-Avatar`](https://github.com/meganetaaan/m5stack-avatar) — animated avatar face

---

## Getting Started

### Prerequisites

- [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html) (CLI)
- An M5Stack Core connected over USB

### Build & Upload

```bash
# Build the firmware
pio run

# Build, upload, and open the serial monitor
pio run --target upload && pio device monitor
```

The board and serial port are preconfigured in [`platformio.ini`](platformio.ini).

---

## Project Structure

```
.
├── platformio.ini      # PlatformIO config (board, libs, serial port)
├── src/
│   └── main.cpp        # Entry point: setup + main loop
├── include/            # Project headers (e.g. secrets.h — git-ignored)
├── lib/                # Private project libraries
├── test/               # Unit tests
├── ROADMAP.md          # Phased feature plan
└── README.md
```

---

## Secrets

WiFi credentials and API tokens live in `include/secrets.h`, which is **not** committed to the
repository (see [`.gitignore`](.gitignore)). Create it from this template:

```cpp
// include/secrets.h
#pragma once

#define WIFI_SSID        "your-network"
#define WIFI_PASSWORD    "your-password"
// #define TELEGRAM_BOT_TOKEN "..."
// #define WEATHER_API_KEY    "..."
```

---

## Status

Early development. Current build shows the avatar with three button-driven expressions.
Track progress against the checklist in [ROADMAP.md](ROADMAP.md).
