#include "Net.h"

#include <WiFi.h>
#include <time.h>

#include "secrets.h"

static bool timeSynced_ = false;
static uint32_t lastReconnectAttempt_ = 0;
static const uint32_t RECONNECT_INTERVAL_MS = 10000;

void Net::begin() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  configTzTime(TZ_POSIX, "pool.ntp.org", "time.nist.gov");
}

void Net::update() {
  uint32_t now = millis();

  if (WiFi.status() != WL_CONNECTED) {
    if (now - lastReconnectAttempt_ >= RECONNECT_INTERVAL_MS) {
      lastReconnectAttempt_ = now;
      WiFi.disconnect();
      WiFi.begin(WIFI_SSID, WIFI_PASS);
    }
    return;
  }

  if (!timeSynced_) {
    struct tm t;
    if (getLocalTime(&t, 0)) {
      timeSynced_ = true;
    }
  }
}

bool Net::isConnected() { return WiFi.status() == WL_CONNECTED; }

bool Net::isTimeSynced() { return timeSynced_; }

int Net::hour() {
  struct tm t;
  if (!getLocalTime(&t, 0)) return -1;
  return t.tm_hour;
}

int Net::minute() {
  struct tm t;
  if (!getLocalTime(&t, 0)) return -1;
  return t.tm_min;
}
