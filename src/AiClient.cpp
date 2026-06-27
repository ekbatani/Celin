#include "AiClient.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <string.h>

#include "secrets.h"

#ifndef CLAUDE_API_KEY
#define CLAUDE_API_KEY ""
#endif

static const char* API_URL = "https://api.anthropic.com/v1/messages";
static const char* MODEL = "claude-haiku-4-5-20251001";
static const char* API_VERSION = "2023-06-01";

static const char* CHAT_SYSTEM_PROMPT =
    "You are Mochi, a friendly playful cat living inside a tiny screen. "
    "You are a child's companion. Rules: "
    "Keep responses under 180 characters. "
    "Be kind, warm, and encouraging. Use simple words a 5-year-old understands. "
    "Never discuss violence, scary topics, or inappropriate content. "
    "Be silly and playful. "
    "If asked something inappropriate, say 'Meow! Let us talk about something fun!'";

static const char* STORY_SYSTEM_PROMPT =
    "You are Mochi, a friendly cat. Tell a very short bedtime story in under 280 "
    "characters. Make it cozy, gentle, and perfect for a young child falling asleep. "
    "Use simple words. End with the characters falling asleep or saying goodnight. "
    "Do not use quotation marks or special characters.";

static const int RESPONSE_BUF_LEN = 512;
static char responseBuf_[RESPONSE_BUF_LEN] = {};

static bool callApi(const char* systemPrompt, const char* userMessage,
                    int maxTokens) {
  if (strlen(CLAUDE_API_KEY) == 0) return false;
  if (WiFi.status() != WL_CONNECTED) return false;

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, API_URL);
  http.setTimeout(15000);
  http.addHeader("Content-Type", "application/json");
  http.addHeader("x-api-key", CLAUDE_API_KEY);
  http.addHeader("anthropic-version", API_VERSION);

  JsonDocument reqDoc;
  reqDoc["model"] = MODEL;
  reqDoc["max_tokens"] = maxTokens;
  reqDoc["system"] = systemPrompt;
  JsonArray messages = reqDoc["messages"].to<JsonArray>();
  JsonObject msg = messages.add<JsonObject>();
  msg["role"] = "user";
  msg["content"] = userMessage;

  String payload;
  serializeJson(reqDoc, payload);

  int code = http.POST(payload);

  bool success = false;
  if (code == 200) {
    String response = http.getString();
    JsonDocument respDoc;
    if (!deserializeJson(respDoc, response)) {
      const char* text = respDoc["content"][0]["text"].as<const char*>();
      if (text) {
        int len = strlen(text);
        if (len > RESPONSE_BUF_LEN - 1) len = RESPONSE_BUF_LEN - 1;
        strncpy(responseBuf_, text, len);
        responseBuf_[len] = '\0';
        success = true;
      }
    }
  }

  http.end();
  return success;
}

void AiClient::begin() { responseBuf_[0] = '\0'; }

bool AiClient::ask(const char* question) {
  return callApi(CHAT_SYSTEM_PROMPT, question, 100);
}

bool AiClient::fetchStory() {
  return callApi(STORY_SYSTEM_PROMPT, "Tell me a bedtime story.", 150);
}

const char* AiClient::getResponse() { return responseBuf_; }
