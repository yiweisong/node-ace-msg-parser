#ifndef UART_MESSAGE_PARSER_H
#define UART_MESSAGE_PARSER_H

#include <napi.h>
#include "types.h"

class MessageParser : public Napi::ObjectWrap<MessageParser> {
  public:
    static void Init(Napi::Env env, Napi::Object exports);
    MessageParser(const Napi::CallbackInfo&);
    Napi::Value Receive(const Napi::CallbackInfo&);
    void Reset(const Napi::CallbackInfo&);

  private:
    std::string _key;
    std::vector<uint16_t> _allowedUserPacketsTypeList;
    std::vector<std::string> _allowedNMEATypeList;
    int _allowedPacketsLength;
    int _allowedNMEAsLength;
    
    ParseStatus _parseStatus;
    void _reset();
    int _accept(uint8_t data);
    int _parse_nmea(uint8_t data);
    int _parse_user_packet_payload(uint8_t* buff, uint32_t nbyte);
    PacketInfo _currentPacket;
};

#endif
