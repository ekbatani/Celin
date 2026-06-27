#ifndef MESSAGING_H_
#define MESSAGING_H_

namespace Messaging {

void begin();
void update();

void sendMessage(const char* text);

bool hasNewMessage();
const char* getLastMessage();
void clearMessage();

}  // namespace Messaging

#endif  // MESSAGING_H_
