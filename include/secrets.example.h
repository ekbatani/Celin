#ifndef SECRETS_H_
#define SECRETS_H_

#define WIFI_SSID "your-ssid"
#define WIFI_PASS "your-password"

// Timezone: POSIX TZ string
// Examples:
//   "CET-1CEST,M3.5.0,M10.5.0/3"  — Central Europe
//   "EST5EDT,M3.2.0,M11.1.0"       — US Eastern
//   "IST-2IDT,M3.4.4/26,M10.5.0"  — Israel
#define TZ_POSIX "CET-1CEST,M3.5.0,M10.5.0/3"

// OpenWeatherMap (free): https://openweathermap.org/api
#define OPENWEATHER_API_KEY "your-api-key"
#define WEATHER_CITY "Amsterdam,NL"

// Telegram bot: https://core.telegram.org/bots#how-do-i-create-a-bot
// 1. Message @BotFather on Telegram, /newbot, follow the prompts.
// 2. Copy the bot token below.
// 3. Send a message to your bot, then visit:
//    https://api.telegram.org/bot<TOKEN>/getUpdates
//    to find your chat_id in the response JSON.
#define TELEGRAM_BOT_TOKEN "123456789:AABBccDDeeFFggHHiiJJkkLLmmNNooP"
#define TELEGRAM_CHAT_ID "123456789"

// Claude API: https://console.anthropic.com/
// Used for bedtime stories and AI Q&A (Phase 5).
// The key is stored on the device — for better security, use a proxy server.
#define CLAUDE_API_KEY "sk-ant-api03-your-key-here"

#endif  // SECRETS_H_
