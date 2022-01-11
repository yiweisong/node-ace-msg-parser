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
    // static inline Napi::Function JSConstructor(Napi::Function *func = nullptr) {
    //     static Napi::FunctionReference constructor;

    //     if (func != nullptr) {
    //         constructor = Napi::Persistent(*func);
    //         constructor.SuppressDestruct();
    //     }
    //     return constructor.Value();
    // }
    std::string _key;
    usrRaw _user_raw;
    void _accept(uint8_t data);
};

#endif
