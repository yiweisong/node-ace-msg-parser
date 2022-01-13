#include "utils.h"
#include "uart_message_parser.h"
#include <iostream>

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

    if (!info[1].IsArray())
    {
        Napi::TypeError::New(env, "Set a allowed packet list")
            .ThrowAsJavaScriptException();
        return;
    }

    this->_user_raw = {0};
    this->_key = info[0].As<Napi::String>().Utf8Value();

    Napi::Array list = info[1].As<Napi::Array>();
    uint32_t allowedPacketsLength = list.Length();
    for(uint32_t i=0;i< allowedPacketsLength;i++){
        this->_userPacketsTypeList.push_back(
            list.Get(i).As<Napi::String>().Utf8Value().c_str()
        );
    }
    this->_allowed_packets_length=this->_userPacketsTypeList.size();
}

Napi::Value UartMessageParser::Receive(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    // buffer
    Napi::Array list = Napi::Array::New(env);
    size_t startIndex = 0;
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
    
    for (size_t i = 0; i < count; i++){
        uint8_t val = buf[i];
        int ret = this->_accept(val);
        if(ret>0){
            Napi::Object packet = Napi::Object::New(env);
            packet.Set(Napi::String::New(env, "packetType"),
                Napi::String::New(env, _currentPacket.packetType, _currentPacket.packetTypeLen));
            // packet.Set(Napi::String::New(env, "payload"),
            //     Napi::Buffer<uint8_t>::New(env, _currentPacket.payload, _currentPacket.payloadLen));
            list.Set(startIndex, packet);
            startIndex++;
        }
    }

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

int UartMessageParser::_parse_nmea(uint8_t data) {
    int ret=0;

    if (_user_raw.nmea_flag == 0) {
        if (NEAM_HEAD == data) {
            _user_raw.nmea_flag = 1;
            _user_raw.nmeabyte = 0;
            _user_raw.nmea[_user_raw.nmeabyte++] = data;
        }
    }
    else if (_user_raw.nmea_flag == 1) {
        _user_raw.nmea[_user_raw.nmeabyte++] = data;
        if (_user_raw.nmeabyte == 6) {
            int i = 0;
            char NMEA[8] = { 0 };
            memcpy(NMEA, _user_raw.nmea, 6);
            for (i = 0; i < MAX_NMEA_TYPES; i++) {
                if (strcmp(NMEA, nmea_type(i)) == 0) {
                    _user_raw.nmea_flag = 2;
                    break;
                }
            }
            if (_user_raw.nmea_flag != 2) {
                _user_raw.nmea_flag = 0;
            }
        }
    }
    else if (_user_raw.nmea_flag == 2) {
        if (is_nmea_char(data)) {
            _user_raw.nmea[_user_raw.nmeabyte++] = data;
        }
        else {
            _user_raw.nmea[_user_raw.nmeabyte++] = 0x0A;
            _user_raw.nmea[_user_raw.nmeabyte++] = 0;
            _user_raw.nmea_flag = 0;

            _currentPacket.packetType = "nmea";
            _currentPacket.packetTypeLen = 4;
            _currentPacket.payload = _user_raw.nmea;
            _currentPacket.payloadLen = sizeof(_user_raw.nmea);
            ret = 2;
        }
    }
    return ret;
}

int UartMessageParser::_parse_user_packet_payload(uint8_t* buff, uint32_t nbyte){
    int ret = 0;

    uint8_t payload_length = buff[2];
    char packetType[4];
    uint8_t payload[payload_length];

    memcpy(packetType, buff, 2);
    memcpy(payload, buff + 3, payload_length);
    
    _currentPacket.packetType = packetType;
    _currentPacket.packetTypeLen = 4;
    _currentPacket.payload = payload;
    _currentPacket.payloadLen = payload_length;
    return 1;    
}

int UartMessageParser::_accept(uint8_t data){
    int ret = 0;
    if (_user_raw.flag == 0) {
		_user_raw.header[_user_raw.header_len++] = data;
		if (_user_raw.header_len == 1) {
			if (_user_raw.header[0] != USER_PREAMB) {
				_user_raw.header_len = 0;
			}
		}
		if (_user_raw.header_len == 2) {
			if (_user_raw.header[1] != USER_PREAMB) {
				_user_raw.header_len = 0;
			}
		}
		if (_user_raw.header_len == 4) {
			int i = 0;
			for (i = 0; i < _allowed_packets_length; i++) {
				const std::string packetType = _userPacketsTypeList.at(i);
				if (packetType[0] == _user_raw.header[2] && packetType[1] == _user_raw.header[3]) {
					_user_raw.flag = 1;
					_user_raw.buff[_user_raw.nbyte++] = packetType[0];
					_user_raw.buff[_user_raw.nbyte++] = packetType[1];
					break;
				}
			}
			_user_raw.header_len = 0;
		}
		ret = _parse_nmea(data);
	}
	else {
		_user_raw.buff[_user_raw.nbyte++] = data;
		if (_user_raw.nbyte == _user_raw.buff[2] + 5) { //5 = [type1,type2,len] + [crc1,crc2]
			uint16_t packet_crc = 256 * _user_raw.buff[_user_raw.nbyte - 2] + _user_raw.buff[_user_raw.nbyte - 1];
			if (packet_crc == calc_crc(_user_raw.buff, _user_raw.nbyte - 2)) {
				ret = _parse_user_packet_payload(_user_raw.buff, _user_raw.nbyte);
			}
			_user_raw.flag = 0;
			_user_raw.nbyte = 0;
		}
	}
	return ret;
}