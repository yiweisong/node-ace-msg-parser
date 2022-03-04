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
    int _allowedPacketsLength;
    int _allowedNMEAsLength;
    bool _skipCheckCRC;

    std::string _key;
    std::vector<uint16_t> _allowedUserPacketsTypeList;
    std::vector<std::string> _allowedNMEATypeList;
    
    ParseStatus _parseStatus;
    PacketInfo _currentPacket;

    void _reset();
    int _accept(uint8_t data);
    int _parse_nmea(uint8_t data);
    int _parse_user_packet_payload(uint8_t* buff, uint32_t nbyte);
    bool _can_calc_binary_payload_len();
};

#endif
