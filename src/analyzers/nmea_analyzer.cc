#include "message_analyzer.h"

namespace Aceinna
{
  size_t MAX_PAYLOAD_SIZE = 128;
  size_t NMEA_PACKET_TYPE_LENGTH = 6;
  
  bool NMEAMessageAnalyzer::isNMEAChar(char c)
  {
    if(c>='A' && c<='Z') {
      return true;
    }
    else if(c>='0' && c<='9') {
      return true;
    }
    else if(c == '$' || c == '-' || c == ',' || c == '.' || c == '*') {
      return true;
    }
    return false;
  }

  bool NMEAMessageAnalyzer::checkPacketType(uint8_t* nmea, std::vector<uint8_t> raw_packet_type, size_t size)
  {
    if(raw_packet_type.size() != size) {
      return false;
    }

    for (size_t i = 0; i < size; i++) {
      if (nmea[i] != raw_packet_type[i]) {
        return false;
      }
    }
    return true;
  }

  NMEAMessageAnalyzer::NMEAMessageAnalyzer(std::vector<PacketType> allow_packet_types)
  {
    m_format = FORMAT_NMEA;
    //m_processStatus = {0};
    m_allowPacketTypes = allow_packet_types;
  }

  void NMEAMessageAnalyzer::determine(uint8_t byte_data, AnalysisResult& analysis_result)
  {
    if (m_processStatus.flag == 0) {
      if(NMEA_HEAD == byte_data) {
        m_processStatus.flag = 1;
        m_processStatus.read_index = 0;
        m_processStatus.nmea[m_processStatus.read_index++] = byte_data;
      }
    }
    else if (m_processStatus.flag == 1) {
      m_processStatus.nmea[m_processStatus.read_index++] = byte_data;
      if(NMEA_HEAD == byte_data) {
        m_processStatus.read_index = 1;
        m_processStatus.nmea[0] = byte_data;
      }
      if (m_processStatus.read_index == NMEA_PACKET_TYPE_LENGTH) {
        
        for (size_t i = 0; i < m_allowPacketTypes.size(); i++) {
          if(checkPacketType(m_processStatus.nmea, m_allowPacketTypes[i].raw, NMEA_PACKET_TYPE_LENGTH))
          {
            m_processStatus.packet_type_id = m_allowPacketTypes[i].id;
            m_processStatus.flag = 2;
            break;
          }
        }
        if (m_processStatus.flag != 2) {
          m_processStatus.flag = 0;
          analysis_result.status = AnalysisStatus::DETERMINING;
        }
        else {
          analysis_result.status = AnalysisStatus::DETERMINED;
          analysis_result.packet_type_id = m_processStatus.packet_type_id;
          
        }
      }
    }
  }

  bool NMEAMessageAnalyzer::canExtract(uint8_t* data, size_t data_size)
  {
    return data_size > MAX_PAYLOAD_SIZE;
  }

  void NMEAMessageAnalyzer::extract(uint8_t* data, size_t data_size, AnalysisResult& analysis_result)
  {
    if (m_processStatus.flag == 2) {
      size_t max_size = data_size < MAX_PAYLOAD_SIZE ? data_size : MAX_PAYLOAD_SIZE;

      for (size_t i = 0; i < max_size; i++) {
        char byte_data = *(data+i);
        if (isNMEAChar(byte_data)) {
          m_processStatus.nmea[m_processStatus.read_index++] = byte_data;
        }
        else {
          m_processStatus.nmea[m_processStatus.read_index++] = 0x0d;
          m_processStatus.nmea[m_processStatus.read_index++] = 0x0a;
          m_processStatus.flag = 0;
          //_parseStatus.read_index = 0;

          analysis_result.status = AnalysisStatus::DONE;
          //analysis_result.packet_type = m_processStatus.packet_type;
          //analysis_result.format_id = 0;
          analysis_result.data = m_processStatus.nmea;
          analysis_result.length = m_processStatus.read_index;
          analysis_result.read_size = m_processStatus.read_index - NMEA_PACKET_TYPE_LENGTH;
          analysis_result.payload_offset = 0;
          analysis_result.payload_len = m_processStatus.read_index;
          break;
        }
      }
    }
  }

  void NMEAMessageAnalyzer::reset()
  {
    m_processStatus.flag = 0;
  }

  std::string &NMEAMessageAnalyzer::name()
  {
    return m_format;
  }
}