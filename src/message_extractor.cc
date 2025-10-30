#include "types.h"
#include "message_extractor.h"
#include <string.h>
#include <stdio.h>

namespace Aceinna {
  MessageExtractor::MessageExtractor(const Napi::CallbackInfo &info)
    : Napi::ObjectWrap<MessageExtractor>(info) {
      Napi::Env env = info.Env();
      Napi::HandleScope scope(env);

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

    Napi::Object options = info[0].As<Napi::Object>();
    if(options.Has("messages")){
        Napi::Array messages = options.Get("messages").As<Napi::Array>();
        for (uint32_t i = 0;i < messages.Length();i++)
        {
          Napi::Object messageFormat = messages.Get(i).As<Napi::Object>();
          std::string format = messageFormat.Get("format").As<Napi::String>().Utf8Value();
          //need to build an acceptable allowPacketTypes
          Napi::Array allowPacketTypes = messageFormat.Get("allowPacketTypes").As<Napi::Array>();
          std::vector<PacketType> packetTypes = {};
          for(size_t m = 0;m < allowPacketTypes.Length();m++)
          {
            Napi::Object packetType = allowPacketTypes.Get(m).As<Napi::Object>();
            uint16_t id = packetType.Get("id").As<Napi::Number>().Int32Value();
            std::string name = packetType.Get("name").As<Napi::String>().Utf8Value();
            Napi::Array raw = packetType.Get("raw").As<Napi::Array>();
            std::vector<uint8_t> rawPacketType = {};

            for(size_t n = 0; n < raw.Length();n++)
            {
              rawPacketType.push_back(raw.Get(n).As<Napi::Number>().Int32Value());  
            }
            packetTypes.push_back({id, name, rawPacketType});
          }
          
          bool skipCheckCRC = messageFormat.Has("skipCheckCRC") ? messageFormat.Get("skipCheckCRC").As<Napi::Boolean>().Value() : true;

          if (format == FORMAT_ACEINNA_BINARY_V1 || format == FORMAT_ACEINNA_BINARY_V2 || format == FORMAT_ACEINNA_BINARY_V3 || format == FORMAT_ACEINNA_BINARY_V4)
          {
            m_messageAnalyzers.push_back(std::make_shared<AceinnaBinaryMessageAnalyzer>(format, packetTypes, skipCheckCRC));
          }

          if (format == FORMAT_NMEA)
          {
            m_messageAnalyzers.push_back(std::make_shared<NMEAMessageAnalyzer>(packetTypes));
          }

          if (format == FORMAT_ACEINNA_BINARY_UDP)
          {
            m_messageAnalyzers.push_back(std::make_shared<HeadlessMessageAnalyzer>(packetTypes));
          }
        }
    }
  }

  Napi::Value MessageExtractor::Receive(const Napi::CallbackInfo &info)
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
    Napi::Buffer<char> buf = info[0].As<Napi::Buffer<char>>();
    size_t actualSize = buf.Length();
    size_t read_index = 0;
    size_t extracted_index = 0;
    //uint8_t* data = (uint8_t*)buf.Data();
    bool resizeActualData = false;
    
    if (m_unparsedDataSize > 0)
    {
        resizeActualData = true;
        //actualData = new uint8_t[m_unparsedDataSize+actualSize];// (uint8_t*)malloc(m_unparsedDataSize + actualSize);
        memcpy(m_actualData, m_unparsedData, m_unparsedDataSize);
        memcpy(m_actualData + m_unparsedDataSize, buf.Data(), actualSize);
        actualSize += m_unparsedDataSize;
    }
    else{
        //printf("11 actual size %zu \n", actualSize);  
        memcpy(m_actualData, buf.Data(), actualSize);
        //m_actualData = buf.Data();
    }
    while (read_index < actualSize)
    {
      //if determining, send data bytes to analyzers one by one
      if (m_analysisResult.status == AnalysisStatus::DETERMINING)
      {
        for (size_t i = 0; i < m_messageAnalyzers.size(); i++)
        {
          m_messageAnalyzers[i]->determine(m_actualData[read_index], m_analysisResult);
          
          if (m_analysisResult.status == AnalysisStatus::DETERMINED)
          {
            m_determinedIndex = i;
            break;
          }
        }

        read_index++;
      }

      //if determined, send batch data to specific analyzer
      if (m_analysisResult.status == AnalysisStatus::DETERMINED)
      {
        size_t left_size = actualSize - read_index;
        //required to invoke determined analyzer's canExtract before extract data
        bool can_extract = m_messageAnalyzers[m_determinedIndex]->canExtract(m_actualData + read_index, left_size);

        if(can_extract){
          m_messageAnalyzers[m_determinedIndex]->extract(m_actualData + read_index, left_size, m_analysisResult);
          _handle_extracted_data(env, list, extracted_index);
          _clear_cross_status();
          extracted_index++;
        }else{
          m_needCross = true;
          m_crossStart = read_index;
          m_crossSize = left_size;
          m_analysisResult.read_size = left_size;
        }
        read_index += m_analysisResult.read_size; //excluded preamble of the determined packet
      }
    }
    //printf("%d %d %d\n", m_analysisResult.status,read_index,actualData[0]);
    
    if(m_needCross && m_crossSize > 0){
      //m_unparsedData = new uint8_t[m_crossSize]; //(uint8_t*)malloc(m_crossSize);
      m_unparsedDataSize = m_crossSize;
      memcpy(m_unparsedData, m_actualData + m_crossStart, m_crossSize);
    }

    if(resizeActualData){
      //free(actualData);
      //delete[] actualData;
    }

    return list;
  }

  void MessageExtractor::Reset(const Napi::CallbackInfo &info)
  {
    _clear_cross_status();
    _reset_message_analyzers();
    _clear_unparsed_data();
  }

  void MessageExtractor::_handle_extracted_data(Napi::Env &env, Napi::Array &list, size_t index)
  {
    Napi::Object packet = Napi::Object::New(env);

    //fill the parsed result to list
    switch(m_analysisResult.status){
      case AnalysisStatus::DONE:
        packet.Set("packetTypeId", m_analysisResult.packet_type_id);
        // packet.Set("counter", m_analysisResult.counter);
        packet.Set("message", Napi::Buffer<uint8_t>::Copy(env, m_analysisResult.data, m_analysisResult.length));
        //packet.Set("length", m_analysisResult.length);
        packet.Set("payloadOffset", m_analysisResult.payload_offset);
        packet.Set("payloadLen", m_analysisResult.payload_len);
        
        list[index] = packet;
        _reset_message_analyzers();
        _clear_unparsed_data();
        break;
      case AnalysisStatus::CRC_ERROR:
        //TODO: handle CRC error
        _reset_message_analyzers();
        _clear_unparsed_data();
        break;
      case AnalysisStatus::FAILED:
        _reset_message_analyzers();
        _clear_unparsed_data();
        break;
      default:
        break;
    }
  }

  void MessageExtractor::_reset_message_analyzers()
  {
    for (size_t i = 0; i < m_messageAnalyzers.size(); i++)
    {
      m_messageAnalyzers[i]->reset();
    }
    m_determinedIndex = -1;

    //m_analysisResult.counter = 0;
    m_analysisResult.packet_type_id = 0;
    m_analysisResult.length = 0;
    m_analysisResult.payload_len = 0;
    m_analysisResult.payload_offset = 0;
    m_analysisResult.status = AnalysisStatus::DETERMINING;
  }

  void MessageExtractor::_clear_cross_status()
  {
    if(m_needCross){
      m_needCross = false;
      m_crossStart = 0;
      m_crossSize = 0;
    }
  }

  void MessageExtractor::_clear_unparsed_data()
  {
    if(m_unparsedDataSize > 0){
      m_unparsedDataSize = 0;
    }

    //if(m_unparsedData != nullptr){
      //free(m_unparsedData);
      //delete m_unparsedData;
      //m_unparsedData = nullptr;
    //}
  }

  void MessageExtractor::Init(Napi::Env env, Napi::Object exports)
  {
    Napi::Function func = DefineClass(
        env,
        "MessageExtractor",//MessageExtractor::JSClassName,
        {
            InstanceMethod("receive", &MessageExtractor::Receive),
            InstanceMethod("reset", &MessageExtractor::Reset),
        });
    //MessageExtractor::JSClassName
    exports.Set("MessageExtractor", func);
  }
}