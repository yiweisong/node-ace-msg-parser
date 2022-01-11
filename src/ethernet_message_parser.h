#ifndef ETHERNET_MESSAGE_PARSER_H
#define ETHERNET_MESSAGE_PARSER_H

#include <napi.h>

class EthernetMessageParser : public Napi::ObjectWrap<EthernetMessageParser> {
 public:
  EthernetMessageParser(const Napi::CallbackInfo&);

  static Napi::Function GetClass(Napi::Env);

 private:
  std::string _key;
};

#endif
