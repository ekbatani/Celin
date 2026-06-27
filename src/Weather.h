#ifndef WEATHER_H_
#define WEATHER_H_

namespace Weather {

void begin();
void update();

bool hasData();
float temperature();
int conditionId();

}  // namespace Weather

#endif  // WEATHER_H_
