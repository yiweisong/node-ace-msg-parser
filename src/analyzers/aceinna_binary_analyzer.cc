#include "../utils.h"
#include "message_analyzer.h"
#include <string>
#include <algorithm>

namespace Aceinna
{
  AceinnaBinaryMessageAnalyzer::AceinnaBinaryMessageAnalyzer(std::string format, std::vector<PacketType> allow_packet_types, bool skip_crc)
  {
    m_format = format;
    //m_processStatus = {0};
    m_skipCRC = skip_crc;
    //m_allowPacketTypes = allow_packet_types;

    if (m_format == FORMAT_ACEINNA_BINARY_V1)
    {
      m_processStatus.format_id = 1;
      m_processStatus.packet_len_type = 1;
      m_processStatus.wrapper_len = 7; // [preamble, packet_type, length](5 bytes) + payload + [crc](2 bytes)
      m_processStatus.payload_offset = 5;
    }

    if (m_format == FORMAT_ACEINNA_BINARY_V2)
    {
      m_processStatus.format_id = 2;
      m_processStatus.packet_len_type = 4;
      m_processStatus.wrapper_len = 10;
      m_processStatus.payload_offset = 8;
    }

    if (m_format == FORMAT_ACEINNA_BINARY_V3)
    {
      m_processStatus.format_id = 3;
      m_processStatus.packet_len_type = 2;
      m_processStatus.wrapper_len = 10;
      m_processStatus.payload_offset = 8;
    }

    for(auto& item: allow_packet_types){
      m_allowPacketTypeIds.push_back(item.id);
    }
  }

  void AceinnaBinaryMessageAnalyzer::determine(uint8_t byte_data, AnalysisResult& analysis_result)
  {
    if (m_processStatus.flag == 0)
    {
      m_processStatus.msg_header[m_processStatus.msg_read_index++] = byte_data;
      if (m_processStatus.msg_read_index == 1) {
        if (m_processStatus.msg_header[0] != USER_PREAMB){
          m_processStatus.msg_read_index = 0;
        }
      }
      if (m_processStatus.msg_read_index == 2) {
        if (m_processStatus.msg_header[1] != USER_PREAMB) {
          m_processStatus.msg_read_index = 0;
        }
      }
      if (m_processStatus.msg_read_index == 4) {
        uint16_t packet_type_id = m_processStatus.msg_header[2] | (m_processStatus.msg_header[3]<< 8);
        if(m_allowPacketTypeIds.end() != std::find(m_allowPacketTypeIds.begin(), m_allowPacketTypeIds.end(), packet_type_id)){
          m_processStatus.flag = 1;
          m_processStatus.packet_type_id = packet_type_id;
          m_processStatus.payload_len = 0;
        }
        
        m_processStatus.msg_read_index = m_processStatus.flag == 1 ? 4 : 0;

        if (m_processStatus.flag == 1)
        {
          analysis_result.status = AnalysisStatus::DETERMINED;
          analysis_result.packet_type_id = m_processStatus.packet_type_id;
          //analysis_result.counter = 0;
          analysis_result.data = nullptr;
          analysis_result.length = 0;
          analysis_result.read_size = 4;
        }
        else {
          if (m_processStatus.msg_header[1] == USER_PREAMB && m_processStatus.msg_header[2] == USER_PREAMB) {
            m_processStatus.msg_header[0] = m_processStatus.msg_header[1];
            m_processStatus.msg_header[1] = m_processStatus.msg_header[2];
            m_processStatus.msg_header[2] = m_processStatus.msg_header[3];
            m_processStatus.msg_read_index = 3;
          }
          else{
            m_processStatus.msg_read_index = 0;
          }
        }
      }
    }
  }

  bool AceinnaBinaryMessageAnalyzer::canExtract(uint8_t* data, size_t data_size)
  {
    if (m_format == FORMAT_ACEINNA_BINARY_V1)
    {
      if (data_size < 1)
      {
        return false;
      }

      uint8_t payload_len = data[0];

      if (data_size < (size_t)payload_len + (size_t)m_processStatus.wrapper_len - 4)
      {
        return false;
      }
      m_processStatus.payload_len = (uint16_t)payload_len;
      //m_processStatus.counter = 0;
      return true;
    }

    if (m_format == FORMAT_ACEINNA_BINARY_V2)
    {
      if (data_size < 4)
      {
        return false;
      }

      uint32_t payload_len = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);

      if (data_size < (size_t)payload_len + (size_t)m_processStatus.wrapper_len - 4)
      {
        return false;
      }
      m_processStatus.payload_len = (uint16_t)payload_len;
      //m_processStatus.counter = 0;
      return true;
    }

    if (m_format == FORMAT_ACEINNA_BINARY_V3)
    {
      if (data_size < 4)
      {
        return false;
      }

      uint16_t payload_len = data[0] | (data[1] << 8);

      if (data_size < (size_t)payload_len + (size_t)m_processStatus.wrapper_len - 4)
      {
        return false;
      }
      m_processStatus.payload_len = payload_len;
      m_processStatus.counter = data[2] | (data[3] << 8);
      return true;
    }

    return false;
  }

  void AceinnaBinaryMessageAnalyzer::extract(uint8_t* data, size_t data_size, AnalysisResult& analysis_result)
  {
    uint16_t whole_packet_len = m_processStatus.payload_len + m_processStatus.wrapper_len;
    memcpy(m_processStatus.msg_buff, m_processStatus.msg_header, m_processStatus.msg_read_index);
    memcpy(m_processStatus.msg_buff + 4, data, whole_packet_len - m_processStatus.msg_read_index);

    if(m_skipCRC)
    {
      //fill analysis_result with DONE, consider refactor as inline function
      analysis_result.status = AnalysisStatus::DONE;
      analysis_result.format_id = m_processStatus.format_id;
      analysis_result.data = m_processStatus.msg_buff;
      analysis_result.length = whole_packet_len;
      analysis_result.payload_offset = m_processStatus.payload_offset;
      analysis_result.payload_len = m_processStatus.payload_len;
      
      m_processStatus.flag = 3;
    } else {
      uint16_t packet_crc = m_processStatus.msg_buff[whole_packet_len - 1] | (m_processStatus.msg_buff[whole_packet_len - 2] << 8);
    
      if(packet_crc == calc_crc(m_processStatus.msg_buff + 2, whole_packet_len - 4))
      {
        //fill analysis_result with DONE, consider refactor as inline function
        analysis_result.status = AnalysisStatus::DONE;
        analysis_result.format_id = m_processStatus.format_id;
        analysis_result.data = m_processStatus.msg_buff;
        analysis_result.length = whole_packet_len;
        analysis_result.payload_offset = m_processStatus.payload_offset;
        analysis_result.payload_len = m_processStatus.payload_len;

        m_processStatus.flag = 3;
      }
      else
      {
        analysis_result.status = AnalysisStatus::CRC_ERROR;
        analysis_result.format_id = m_processStatus.format_id;
        analysis_result.data = m_processStatus.msg_buff;
        analysis_result.length = whole_packet_len;
        //fill analysis_result with CRC ERROR
        
        m_processStatus.flag = 3;
      }
    }
    analysis_result.read_size = whole_packet_len - m_processStatus.msg_read_index;

    if(m_processStatus.flag == 3)
    {
      m_processStatus.flag = 0;
      m_processStatus.msg_read_index = 0;
    }
  }

  void AceinnaBinaryMessageAnalyzer::reset()
  {
    m_processStatus.flag = 0;
    m_processStatus.msg_read_index = 0;
  }

  std::string &AceinnaBinaryMessageAnalyzer::name()
  {
    return m_format;
  }
}