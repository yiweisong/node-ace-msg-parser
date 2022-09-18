#include "utils.h"
#include "message_parser.h"
#include <iostream>
#include <string.h>

using namespace Napi;

MessageParser::MessageParser(const Napi::CallbackInfo &info)
    : ObjectWrap(info)
{
    Napi::Env env = info.Env();
    
    if (info.Length() < 1)
    {
        Napi::TypeError::New(env, "Wrong number of arguments")
            .ThrowAsJavaScriptException();
        return;
    }

    if (!info[0].IsObject())
    {
        Napi::TypeError::New(env, "Set an options for the parser")
            .ThrowAsJavaScriptException();
        return;
    }

    this->_reset();
    
    Napi::Object options = info[0].As<Napi::Object>();
    if(options.Has("key")){
        this->_key = options.Get("key").As<Napi::String>().Utf8Value();
    }
    
    if(options.Has("user")){
        Napi::Object userOptions = options.Get("user").As<Napi::Object>();

        if(userOptions.Has("allows")){
            Napi::Array list = userOptions.Get("allows").As<Napi::Array>();
                        
            uint32_t allowedPacketsLength = list.Length();

            for(uint32_t i=0;i< allowedPacketsLength;i++)
            {
                
                this->_allowedUserPacketsTypeList.push_back(
                    list.Get(i).As<Napi::Number>().Uint32Value()
                );
            }
            this->_allowedPacketsLength=allowedPacketsLength;
        }
        if(userOptions.Has("packetLengthType")){
            std::string packetLengthType = userOptions.Get("packetLengthType").As<Napi::String>().Utf8Value();
            if(packetLengthType == "uint8")
            {
                _parseStatus.binary_packet_len_type = 1;
                // 2 preamble + 2 packet type + 1 packet len + 2 crc
                _parseStatus.binary_wrapper_len = 7;
            } 
            else if(packetLengthType == "uint32")
            {
                _parseStatus.binary_packet_len_type = 4;
                // 2 preamble + 2 packet type + 4 packet len + 2 crc
                _parseStatus.binary_wrapper_len = 10;
            } 
            else
            {
                _parseStatus.binary_packet_len_type = 1;
                _parseStatus.binary_wrapper_len = 7;
            }
        }
    }

    if(options.Has("nmea")){
        Napi::Object nmeaOptions = options.Get("nmea").As<Napi::Object>();
        if(nmeaOptions.Has("allows")){
            Napi::Array list = nmeaOptions.Get("allows").As<Napi::Array>();
            uint32_t allowedPacketsLength = list.Length();
            for(uint32_t i=0;i< allowedPacketsLength;i++){
                this->_allowedNMEATypeList.push_back(
                    list.Get(i).As<Napi::String>().Utf8Value()
                );
            }
            this->_allowedNMEAsLength=allowedPacketsLength;
        }
    }

    if(options.Has("skipCheckCRC")){
        this->_skipCheckCRC = options.Get("skipCheckCRC").As<Napi::Boolean>().Value();
    }else{
        this->_skipCheckCRC = false;
    }
}

Napi::Value MessageParser::Receive(const Napi::CallbackInfo &info)
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
            Napi::Object packet = Napi::Object::New(env);

            // parse as user packet
            if(ret==1){
                packet.Set(Napi::String::New(env, "packetType"),
                    Napi::Number::New(env, _parseStatus.binary_packet_type));

                packet.Set(Napi::String::New(env, "payload"),
                    Napi::Buffer<uint8_t>::Copy(env, _parseStatus.binary_msg_buff, _currentPacket.binary_msg_len));    
            }

            // parse as nmea packet
            if(ret==2){
                packet.Set(Napi::String::New(env, "packetType"),
                    Napi::Number::New(env, 0x6e4d)); // nmea -> nM

                packet.Set(Napi::String::New(env, "payload"),
                    Napi::Buffer<uint8_t>::Copy(env, _parseStatus.nmea_msg_buff, _currentPacket.nmea_msg_len));
            }
            
            list.Set(startIndex, packet);
            startIndex++;
        }
    }

    return list;
}

void MessageParser::Reset(const Napi::CallbackInfo&)
{
    this->_reset();
}

void MessageParser::Init(Napi::Env env, Napi::Object exports)
{
    Napi::Function func = DefineClass(
        env,
        "MessageParser",
        {
            InstanceMethod("receive", &MessageParser::Receive),
            InstanceMethod("reset", &MessageParser::Reset),
        });
    exports.Set("MessageParser", func);
}

int MessageParser::_parse_nmea(uint8_t data) {
    int ret=0;

    if (_parseStatus.nmea_flag == 0) {
        if (NEAM_HEAD == data) {
            _parseStatus.nmea_flag = 1;
            _parseStatus.nmea_msg_len = 0;
            _parseStatus.nmea_msg_buff[_parseStatus.nmea_msg_len++] = data;
        }
    }
    else if (_parseStatus.nmea_flag == 1) {
        _parseStatus.nmea_msg_buff[_parseStatus.nmea_msg_len++] = data;
        if (_parseStatus.nmea_msg_len == 6) {
            int i = 0;
            char NMEA[8] = { 0 };
            memcpy(NMEA, _parseStatus.nmea_msg_buff, 6);
            for (i = 0; i < _allowedNMEAsLength; i++) {
                std::string nmeaType = _allowedNMEATypeList.at(i);
                if (nmeaType.compare(NMEA) == 0) {
                    _parseStatus.nmea_flag = 2;
                    break;
                }
            }
            if (_parseStatus.nmea_flag != 2) {
                _parseStatus.nmea_flag = 0;
            }
        }
    }
    else if (_parseStatus.nmea_flag == 2) {
        if (is_nmea_char(data)) {
            _parseStatus.nmea_msg_buff[_parseStatus.nmea_msg_len++] = data;
        }
        else {
            _parseStatus.nmea_msg_buff[_parseStatus.nmea_msg_len++] = 0x0d;
            _parseStatus.nmea_msg_buff[_parseStatus.nmea_msg_len++] = 0x0a;
            _parseStatus.nmea_flag = 0;

            //char packetType[5] = "nmea";
            //_currentPacket.packet_type = packetType;
            //_currentPacket.payload_len = _userRaw.nmeabyte;
            _currentPacket.nmea_msg_len = _parseStatus.nmea_msg_len;
            ret = 2;
        }
    }
    return ret;
}

int MessageParser::_accept(uint8_t data){
    int ret = 0;
    if (_parseStatus.binary_flag == 0) {
		_parseStatus.binary_msg_header[_parseStatus.binary_msg_read_index++] = data;
		if (_parseStatus.binary_msg_read_index == 1) {
			if (_parseStatus.binary_msg_header[0] != USER_PREAMB) {
				_parseStatus.binary_msg_read_index = 0;
			}
		}
		if (_parseStatus.binary_msg_read_index == 2) {
			if (_parseStatus.binary_msg_header[1] != USER_PREAMB) {
				_parseStatus.binary_msg_read_index = 0;
			}
		}
		if (_parseStatus.binary_msg_read_index == 4) {
            // uint16_t binaryPacketType = ((_parseStatus.binary_msg_header[2] & 0xffff) << 8)
            //         | _parseStatus.binary_msg_header[3];
            uint16_t binaryPacketType = _parseStatus.binary_msg_header[2] * 256 + _parseStatus.binary_msg_header[3];
			for (int i = 0; i < _allowedPacketsLength; i++)
            {
				const uint16_t packetType = _allowedUserPacketsTypeList.at(i);
				if (binaryPacketType == packetType) 
                {
					_parseStatus.binary_flag = 1;
                    _parseStatus.binary_msg_buff[0] = USER_PREAMB;
                    _parseStatus.binary_msg_buff[1] = USER_PREAMB;
					_parseStatus.binary_msg_buff[2] = _parseStatus.binary_msg_header[2];
					_parseStatus.binary_msg_buff[3] = _parseStatus.binary_msg_header[3];
                    _parseStatus.binary_packet_type = packetType;
                    _parseStatus.binary_payload_len = 0;
					break;
				}
			}
            _parseStatus.binary_msg_read_index = _parseStatus.binary_flag ? 4 : 0;

            //move binary_msg_read_index to previous. check preamb, if match continue,else move again, until to 0
            if(_parseStatus.binary_flag){
                _parseStatus.binary_msg_read_index=4;
            }else{
                //move_binary_msg_read_index
                if(_parseStatus.binary_msg_header[1]==USER_PREAMB && _parseStatus.binary_msg_header[2]==USER_PREAMB){
                    _parseStatus.binary_msg_header[0]=_parseStatus.binary_msg_header[1];
                    _parseStatus.binary_msg_header[1]=_parseStatus.binary_msg_header[2];
                    _parseStatus.binary_msg_header[2]=_parseStatus.binary_msg_header[3];
                    _parseStatus.binary_msg_read_index=3;
                }

                else if(_parseStatus.binary_msg_header[2]==USER_PREAMB && _parseStatus.binary_msg_header[3]==USER_PREAMB){
                    _parseStatus.binary_msg_header[0]=_parseStatus.binary_msg_header[2];
                    _parseStatus.binary_msg_header[1]=_parseStatus.binary_msg_header[3];
                    _parseStatus.binary_msg_read_index=2;
                }
                else{
                    _parseStatus.binary_msg_read_index=0;
                }
            }
		}
		ret = _parse_nmea(data);
	}
	else {
		_parseStatus.binary_msg_buff[_parseStatus.binary_msg_read_index++] = data;
        
        if(_parseStatus.binary_flag == 1)
        {
            if(this->_can_calc_binary_payload_len())
            {
                _parseStatus.binary_flag = 2;
            }
        }

        if(_parseStatus.binary_flag == 2)
        {
            if (_parseStatus.binary_msg_read_index == _parseStatus.binary_payload_len + _parseStatus.binary_wrapper_len) 
            {
                if(this->_skipCheckCRC)
                {
                    _currentPacket.binary_msg_len = _parseStatus.binary_msg_read_index;
                    ret = 1;
                }
                else
                {
                    uint16_t packet_crc = 256 * _parseStatus.binary_msg_buff[_parseStatus.binary_msg_read_index - 2] 
                        + _parseStatus.binary_msg_buff[_parseStatus.binary_msg_read_index - 1];
                    // skip preamble
                    uint8_t *skipPreambleMsgBuff = _parseStatus.binary_msg_buff;
                    if (packet_crc == calc_crc(skipPreambleMsgBuff + 2, _parseStatus.binary_msg_read_index - 4)) {
                        _currentPacket.binary_msg_len = _parseStatus.binary_msg_read_index;
                        ret = 1;
                    } else{
                        //TODO: collect crc error
                        std::cerr<<"crc error "<< _parseStatus.binary_packet_type<<" "<<_parseStatus.binary_payload_len<<std::endl;
                        // for(int i=0;i<_parseStatus.binary_msg_read_index;i++){
                        //     printf("%d ",_parseStatus.binary_msg_buff[i]);
                        // }
                        // printf("\n");
                    }
                }
                _parseStatus.binary_flag = 0;
                _parseStatus.binary_msg_read_index = 0;
            }
        }
	}
	return ret;
}

bool MessageParser::_can_calc_binary_payload_len()
{
    bool can_calc = _parseStatus.binary_msg_read_index - 4 == _parseStatus.binary_packet_len_type;

    if(can_calc)
    {   
        uint32_t base = 1;
        uint8_t startIndex = 4;
        for(int i = 0; i < _parseStatus.binary_packet_len_type; i++)
        {
            if(i == 0)
            {
                base = 1;
            }
            else
            {
                base *= 256;
            }
            uint8_t partLen = _parseStatus.binary_msg_buff[startIndex+i];
            _parseStatus.binary_payload_len += base*partLen;
        }
        //printf("%d %d\n",_parseStatus.binary_packet_type ,_parseStatus.binary_payload_len);
    }

    return can_calc;
}

void MessageParser::_reset(){
    _parseStatus.binary_flag = 0;
    _parseStatus.binary_msg_read_index = 0;
    _parseStatus.nmea_flag = 0;
    _parseStatus.nmea_msg_len = 0;

    this->_allowedPacketsLength = 0;
    this->_allowedNMEAsLength = 0;
}