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

    this->_userRaw = {0};
    this->_key = info[0].As<Napi::String>().Utf8Value();

    Napi::Array list = info[1].As<Napi::Array>();
    uint32_t allowedPacketsLength = list.Length();
    for(uint32_t i=0;i< allowedPacketsLength;i++){
        this->_userPacketsTypeList.push_back(
            list.Get(i).As<Napi::String>().Utf8Value().c_str()
        );
    }
    this->_allowedPacketsLength=allowedPacketsLength;
}

Napi::Value UartMessageParser::Receive(const Napi::CallbackInfo &info)
{
    Napi::Env env = info.Env();
    Napi::Array list = Napi::Array::New(env);

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

    size_t startIndex = 0;
    Napi::Buffer<char> buf = info[0].As<Napi::Buffer<char>>();
    size_t count = buf.Length();
    
    for (size_t i = 0; i < count; i++){
        uint8_t val = buf[i];
        int ret = this->_accept(val);
        if(ret>0){
            std::string packetTypeStr(_currentPacket.packet_type);
            Napi::Object packet = Napi::Object::New(env);

            packet.Set(Napi::String::New(env, "packetType"),
                Napi::String::New(env, packetTypeStr));

            // parse as user packet
            if(ret==1){
                packet.Set(Napi::String::New(env, "payload"),
                    Napi::Buffer<uint8_t>::Copy(env, _userRaw.buff+3, _currentPacket.payload_len));    
            }

            // parse as nmea packet
            if(ret==2){
                packet.Set(Napi::String::New(env, "payload"),
                    Napi::Buffer<uint8_t>::Copy(env, _userRaw.nmea, _currentPacket.payload_len));
            }
            
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

    if (_userRaw.nmea_flag == 0) {
        if (NEAM_HEAD == data) {
            _userRaw.nmea_flag = 1;
            _userRaw.nmeabyte = 0;
            _userRaw.nmea[_userRaw.nmeabyte++] = data;
        }
    }
    else if (_userRaw.nmea_flag == 1) {
        _userRaw.nmea[_userRaw.nmeabyte++] = data;
        if (_userRaw.nmeabyte == 6) {
            int i = 0;
            char NMEA[8] = { 0 };
            memcpy(NMEA, _userRaw.nmea, 6);
            for (i = 0; i < MAX_NMEA_TYPES; i++) {
                if (strcmp(NMEA, nmea_type(i)) == 0) {
                    _userRaw.nmea_flag = 2;
                    break;
                }
            }
            if (_userRaw.nmea_flag != 2) {
                _userRaw.nmea_flag = 0;
            }
        }
    }
    else if (_userRaw.nmea_flag == 2) {
        if (is_nmea_char(data)) {
            _userRaw.nmea[_userRaw.nmeabyte++] = data;
        }
        else {
            _userRaw.nmea[_userRaw.nmeabyte++] = 0x0A;
            _userRaw.nmea[_userRaw.nmeabyte++] = 0;
            _userRaw.nmea_flag = 0;

            char packetType[5] = "nmea";
            _currentPacket.packet_type = packetType;
            _currentPacket.payload_len = _userRaw.nmeabyte;
            ret = 2;
        }
    }
    return ret;
}

int UartMessageParser::_parse_user_packet_payload(uint8_t* buff, uint32_t nbyte){
    int ret = 1;

    uint8_t payload_length = buff[2];
    char packetType[4];
    uint8_t payload[payload_length];

    memcpy(packetType, buff, 2);
    memcpy(payload, buff + 3, payload_length);
    
    _currentPacket.packet_type = packetType;
    _currentPacket.payload_len = payload_length;

    return ret;
}

int UartMessageParser::_accept(uint8_t data){
    int ret = 0;
    if (_userRaw.flag == 0) {
		_userRaw.header[_userRaw.header_len++] = data;
		if (_userRaw.header_len == 1) {
			if (_userRaw.header[0] != USER_PREAMB) {
				_userRaw.header_len = 0;
			}
		}
		if (_userRaw.header_len == 2) {
			if (_userRaw.header[1] != USER_PREAMB) {
				_userRaw.header_len = 0;
			}
		}
		if (_userRaw.header_len == 4) {
			int i = 0;
			for (i = 0; i < _allowedPacketsLength; i++) {
				const std::string packetType = _userPacketsTypeList.at(i);
				if (packetType[0] == _userRaw.header[2] && packetType[1] == _userRaw.header[3]) {
					_userRaw.flag = 1;
					_userRaw.buff[_userRaw.nbyte++] = packetType[0];
					_userRaw.buff[_userRaw.nbyte++] = packetType[1];
					break;
				}
			}
			_userRaw.header_len = 0;
		}
		ret = _parse_nmea(data);
	}
	else {
		_userRaw.buff[_userRaw.nbyte++] = data;
		if (_userRaw.nbyte == _userRaw.buff[2] + 5) { //5 = [type1,type2,len] + [crc1,crc2]
			uint16_t packet_crc = 256 * _userRaw.buff[_userRaw.nbyte - 2] + _userRaw.buff[_userRaw.nbyte - 1];
			if (packet_crc == calc_crc(_userRaw.buff, _userRaw.nbyte - 2)) {
				ret = _parse_user_packet_payload(_userRaw.buff, _userRaw.nbyte);
			}
			_userRaw.flag = 0;
			_userRaw.nbyte = 0;
		}
	}
	return ret;
}