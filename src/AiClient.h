#ifndef AI_CLIENT_H_
#define AI_CLIENT_H_

namespace AiClient {

void begin();

// Blocking calls to Claude API. Return true on success.
bool ask(const char* question);
bool fetchStory();

const char* getResponse();

}  // namespace AiClient

#endif  // AI_CLIENT_H_
