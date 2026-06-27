#ifndef NET_H_
#define NET_H_

#include <cstdint>

namespace Net {

void begin();

// Call from loop(); handles reconnection. Non-blocking.
void update();

bool isConnected();
bool isTimeSynced();

// Current hour (0-23) and minute (0-59). Only valid when isTimeSynced().
int hour();
int minute();

}  // namespace Net

#endif  // NET_H_
