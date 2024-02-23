#include <napi.h>
#include "message_extractor.h"

//#include "ethernet_message_parser.h"

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  Aceinna::MessageExtractor::Init(env, exports);
  return exports;
}

NODE_API_MODULE(acemsgparser, Init);
