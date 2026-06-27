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

#endif  // SECRETS_H_
