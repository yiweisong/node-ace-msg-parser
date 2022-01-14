#ifndef ETHERNET_MESSAGE_PARSER_H
#define ETHERNET_MESSAGE_PARSER_H

#include <napi.h>
#include "types.h"

class EthernetMessageParser : public Napi::ObjectWrap<EthernetMessageParser> {
  public:
    static void Init(Napi::Env env, Napi::Object exports);
    EthernetMessageParser(const Napi::CallbackInfo&);
    Napi::Value Receive(const Napi::CallbackInfo&);
  
 private:
    std::string _key;
    std::vector<std::string> _userPacketsTypeList;
    int _allowedPacketsLength;
    usrRaw _userRaw;
    int _accept(uint8_t data);
    int _parse_nmea(uint8_t data);
    int _parse_user_packet_payload(uint8_t* buff, uint32_t nbyte);
    PacketInfo _currentPacket;
};

#endif
