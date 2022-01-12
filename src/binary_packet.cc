#include "binary_packet.h"
#include <stdio.h>

using namespace Napi;

BinaryPacket::BinaryPacket(const Napi::CallbackInfo &info)
    : ObjectWrap(info)
{
    printf("%s\n","test");
    Napi::Env env = info.Env();

    // if (info.Length() < 2)
    // {
    //     Napi::TypeError::New(env, "Wrong number of arguments")
    //         .ThrowAsJavaScriptException();
    //     return;
    // }

    // if (!info[0].IsString())
    // {
    //     Napi::TypeError::New(env, "Packet type should be a string")
    //         .ThrowAsJavaScriptException();
    //     return;
    // }

    // if (!info[1].IsBuffer())
    // {
    //     Napi::TypeError::New(env, "Payload should be buffer")
    //         .ThrowAsJavaScriptException();
    //     return;
    // }

    //this->packetType_ = info[0].As<Napi::String>();
    //this->payload_ = info[1].As<Napi::Buffer<char>>();
}

Napi::Value BinaryPacket::GetPacketType(const Napi::CallbackInfo &info) {
    return this->packetType_;
}

Napi::Value BinaryPacket::GetPayload(const Napi::CallbackInfo &info) {
    return this->payload_;
}

Napi::Object BinaryPacket::NewInstance(Napi::Env env, std::string packetType){
    Napi::EscapableHandleScope scope(env);
    Napi::Object wrap = Napi::Object::New(env);
    //JSConstructor().New({Napi::String::New(env, packetType)});
    JSConstructor().New({});
    // .New(
    //     {
    //         Napi::String::New(env, packetType)
    //         //Napi::Buffer<char>::New(env, {0}, sizeof(1)),
    //     }
    // );
    return scope.Escape(napi_value(wrap)).ToObject();
}

void BinaryPacket::Init(Napi::Env env, Napi::Object exports)
{
    Napi::HandleScope scope(env);

    Napi::Function func = DefineClass(
        env,
        "BinaryPacket",
        {
            InstanceAccessor<&BinaryPacket::GetPacketType>("packetType"),
            InstanceAccessor<&BinaryPacket::GetPayload>("payload"),
        });
    //JSConstructor(&func);
    exports.Set("BinaryPacket", func);
}