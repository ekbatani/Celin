#include "Weather.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <string.h>

#include "secrets.h"

#ifndef OPENWEATHER_API_KEY
#define OPENWEATHER_API_KEY ""
#endif
#ifndef WEATHER_CITY
#define WEATHER_CITY ""
#endif

static float temp_ = 0;
static int conditionId_ = 0;
static bool hasData_ = false;
static uint32_t lastFetch_ = 0;
static bool firstFetch_ = true;

static const uint32_t FETCH_INTERVAL = 7200000;   // 2 hours
static const uint32_t FIRST_FETCH_DELAY = 30000;   // 30s after boot (WiFi settle)

void Weather::begin() {
  lastFetch_ = millis();
}

void Weather::update() {
  if (strlen(OPENWEATHER_API_KEY) == 0 || strlen(WEATHER_CITY) == 0) return;

  uint32_t now = millis();

  if (firstFetch_) {
    if (now - lastFetch_ < FIRST_FETCH_DELAY) return;
  } else {
    if (now - lastFetch_ < FETCH_INTERVAL) return;
  }

  if (WiFi.status() != WL_CONNECTED) return;

  lastFetch_ = now;
  firstFetch_ = false;

  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/weather?q=";
  url += WEATHER_CITY;
  url += "&appid=";
  url += OPENWEATHER_API_KEY;
  url += "&units=metric";

  http.begin(url);
  http.setTimeout(5000);
  int code = http.GET();

  if (code == 200) {
    String payload = http.getString();
    JsonDocument doc;
    if (!deserializeJson(doc, payload)) {
      temp_ = doc["main"]["temp"].as<float>();
      conditionId_ = doc["weather"][0]["id"].as<int>();
      hasData_ = true;
    }
  }

  http.end();
}

bool Weather::hasData() { return hasData_; }
float Weather::temperature() { return temp_; }
int Weather::conditionId() { return conditionId_; }
