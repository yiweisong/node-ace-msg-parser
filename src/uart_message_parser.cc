#include "binary_packet.h"
#include "uart_message_parser.h"

using namespace Napi;

UartMessageParser::UartMessageParser(const Napi::CallbackInfo &info)
    : ObjectWrap(info)
{
    Napi::Env env = info.Env();

    if (info.Length() < 1)
    {
        Napi::TypeError::New(env, "Wrong number of arguments")
            .ThrowAsJavaScriptException();
        return;
    }

    if (!info[0].IsString())
    {
        Napi::TypeError::New(env, "Set a key for the parser")
            .ThrowAsJavaScriptException();
        return;
    }
    this->_user_raw = {0};
    this->_key = info[0].As<Napi::String>().Utf8Value();
}

Napi::Value UartMessageParser::Receive(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    // buffer
    Napi::Array list = Napi::Array::New(env);
    int startIndex = 0;
    if (info.Length() != 1) {
        Napi::Error::New(info.Env(), "Expected exactly one argument")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }

    if (!info[0].IsBuffer()) {
        Napi::Error::New(info.Env(), "Expected an Buffer")
            .ThrowAsJavaScriptException();
        return info.Env().Undefined();
    }

    Napi::Buffer<char> buf = info[0].As<Napi::Buffer<char>>();
    size_t count = buf.Length();
    
    // for (size_t i = 0; i < count; i++){
    //     uint8_t val = buf[i];
    //     BinaryPacket binaryPacket = this->_accept(val, this->_user_raw);
    //     if(binaryPacket){
    //         list.set(startIndex, binaryPacket);
    //         startIndex++;
    //     }
    // }

    auto binaryPacket = BinaryPacket::NewInstance(env,"test");
    list.Set(startIndex,binaryPacket);

    return list;
}

void UartMessageParser::Init(Napi::Env env, Napi::Object exports)
{
    Napi::Function func = DefineClass(
        env,
        "UartMessageParser",
        {
            InstanceMethod("receive", &UartMessageParser::Receive),
        });
    //JSConstructor(&func);
    exports.Set("UartMessageParser", func);
}

// void UartMessageParser::_accept(uint8_t data, usrRaw& user_raw){
//     int ret = 0;
// 	if (user_raw.flag == 0) {
// 		user_raw.header[user_raw.header_len++] = data;
// 		if (user_raw.header_len == 1) {
// 			if (user_raw.header[0] != USER_PREAMB) {
// 				user_raw.header_len = 0;
// 			}
// 		}
// 		if (user_raw.header_len == 2) {
// 			if (user_raw.header[1] != USER_PREAMB) {
// 				user_raw.header_len = 0;
// 			}
// 		}
// 		if (user_raw.header_len == 4) {
// 			int i = 0;
// 			for (i = 0; i < MAX_PACKET_TYPES; i++) {
// 				const char* packetType = userPacketsTypeList[i];
// 				if (packetType[0] == user_raw.header[2] && packetType[1] == user_raw.header[3]) {
// 					user_raw.flag = 1;
// 					user_raw.buff[user_raw.nbyte++] = packetType[0];
// 					user_raw.buff[user_raw.nbyte++] = packetType[1];
// 					break;
// 				}
// 			}
// 			user_raw.header_len = 0;
// 		}
// 		//return parse_user_nmea(data);
// 	}
// 	else {
// 		user_raw.buff[user_raw.nbyte++] = data;
// 		if (user_raw.nbyte == user_raw.buff[2] + 5) { //5 = [type1,type2,len] + [crc1,crc2]
// 			uint16_t packet_crc = 256 * user_raw.buff[user_raw.nbyte - 2] + user_raw.buff[user_raw.nbyte - 1];
// 			if (packet_crc == calc_crc(user_raw.buff, user_raw.nbyte - 2)) {
// 				//parse_user_packet_payload(user_raw.buff, user_raw.nbyte);
// 				ret = 1;
// 			}
// 			user_raw.flag = 0;
// 			user_raw.nbyte = 0;
// 		}
// 	}
// 	//return ret;
// }