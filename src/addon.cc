#include <napi.h>
//#include "binary_packet.h"
#include "uart_message_parser.h"

//#include "ethernet_message_parser.h"

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);
  UartMessageParser::Init(env, exports);
  //BinaryPacket::Init(env,exports);
  return exports;
}

NODE_API_MODULE(addon, Init)
