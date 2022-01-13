#ifndef UART_MESSAGE_PARSER_H
#define UART_MESSAGE_PARSER_H

#include <napi.h>
#include "types.h"

class UartMessageParser : public Napi::ObjectWrap<UartMessageParser> {
  public:
    static void Init(Napi::Env env, Napi::Object exports);
    UartMessageParser(const Napi::CallbackInfo&);
    Napi::Value Receive(const Napi::CallbackInfo&);

  private:
    std::string _key;
    std::vector<std::string> _userPacketsTypeList;
    int _allowed_packets_length;
    usrRaw _user_raw;
    int _accept(uint8_t data);
    int _parse_nmea(uint8_t data);
    int _parse_user_packet_payload(uint8_t* buff, uint32_t nbyte);
    UartBinaryPacket _currentPacket;
};

#endif
