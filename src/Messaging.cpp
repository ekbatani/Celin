#include "Messaging.h"

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#include "secrets.h"

#ifndef TELEGRAM_BOT_TOKEN
#define TELEGRAM_BOT_TOKEN ""
#endif
#ifndef TELEGRAM_CHAT_ID
#define TELEGRAM_CHAT_ID ""
#endif

static WiFiClientSecure secureClient;
static UniversalTelegramBot* bot = nullptr;

static uint32_t lastPoll_ = 0;
static bool firstPoll_ = true;
static const uint32_t POLL_INTERVAL = 10000;
static const uint32_t FIRST_POLL_DELAY = 15000;

static const int MSG_BUF_LEN = 200;
static char lastMsg_[MSG_BUF_LEN] = {};
static bool newMessage_ = false;

void Messaging::begin() {
  if (strlen(TELEGRAM_BOT_TOKEN) == 0) return;

  secureClient.setInsecure();
  bot = new UniversalTelegramBot(TELEGRAM_BOT_TOKEN, secureClient);
  bot->longPoll = 0;
  lastPoll_ = millis();
}

void Messaging::update() {
  if (!bot) return;
  if (WiFi.status() != WL_CONNECTED) return;

  uint32_t now = millis();

  if (firstPoll_) {
    if (now - lastPoll_ < FIRST_POLL_DELAY) return;
    firstPoll_ = false;
  } else {
    if (now - lastPoll_ < POLL_INTERVAL) return;
  }

  lastPoll_ = now;

  int count = bot->getUpdates(bot->last_message_received + 1);

  for (int i = 0; i < count; i++) {
    String chatId = bot->messages[i].chat_id;
    if (chatId != TELEGRAM_CHAT_ID) continue;

    String text = bot->messages[i].text;
    int len = text.length();
    if (len > MSG_BUF_LEN - 1) len = MSG_BUF_LEN - 1;
    text.toCharArray(lastMsg_, len + 1);
    newMessage_ = true;
  }
}

void Messaging::sendMessage(const char* text) {
  if (!bot) return;
  if (WiFi.status() != WL_CONNECTED) return;

  bot->sendMessage(TELEGRAM_CHAT_ID, text, "");
}

bool Messaging::hasNewMessage() { return newMessage_; }
const char* Messaging::getLastMessage() { return lastMsg_; }
void Messaging::clearMessage() { newMessage_ = false; }
