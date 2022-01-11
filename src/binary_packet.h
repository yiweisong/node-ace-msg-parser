#ifndef BINARY_PACKET_H
#define BINARY_PACKET_H

#include <napi.h>

class BinaryPacket : public Napi::ObjectWrap<BinaryPacket> {
  public:
    static void Init(Napi::Env env, Napi::Object exports);

    explicit BinaryPacket(const Napi::CallbackInfo&);

    static Napi::Object NewInstance(Napi::Env env, std::string packetType);
  private:
    //static constexpr const char JSClassName[] = "BinaryPacket";

    static inline Napi::Function JSConstructor(Napi::Function *func = nullptr) {
      static Napi::FunctionReference constructor;

      if (func != nullptr) {
          constructor = Napi::Persistent(*func);
          constructor.SuppressDestruct();
      }
      return constructor.Value();
    }

    Napi::String packetType_;

    Napi::Buffer<char> payload_;

    Napi::Value GetPacketType(const Napi::CallbackInfo &info);

    Napi::Value GetPayload(const Napi::CallbackInfo &info);
};

#endif