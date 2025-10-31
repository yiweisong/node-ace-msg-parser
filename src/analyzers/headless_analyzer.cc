#include "message_analyzer.h"

namespace Aceinna {
  HeadlessMessageAnalyzer::HeadlessMessageAnalyzer(std::vector<PacketType> allow_packet_types)
  {
    m_format = FORMAT_ACEINNA_BINARY_UDP;
    m_allowPacketTypeIds = {};
    for(auto& item: allow_packet_types){
      m_allowPacketTypeIds.push_back(item.id);
    }

    m_processStatus.flag = 0;
  }
  
  void HeadlessMessageAnalyzer::determine(uint8_t byte_data, AnalysisResult& analysis_result)
  {
    if(m_processStatus.flag == 0){
      m_processStatus.packet_type_check_list[m_processStatus.msg_read_index++] = byte_data;

      if(m_processStatus.msg_read_index == 2){
        uint16_t packet_type_id = m_processStatus.packet_type_check_list[0] | (m_processStatus.packet_type_check_list[1]<<8);
        if(std::find(m_allowPacketTypeIds.begin(), m_allowPacketTypeIds.end(), packet_type_id) != m_allowPacketTypeIds.end()){
          m_processStatus.flag = 1;
          analysis_result.status = AnalysisStatus::DETERMINED;
          analysis_result.packet_type_id = packet_type_id;
          analysis_result.data = nullptr;
          analysis_result.length = 0;
          analysis_result.read_size = 2;
        } else {
          m_processStatus.packet_type_check_list[0] = m_processStatus.packet_type_check_list[1];
          m_processStatus.msg_read_index = 1;
        }
      }
    }
  }

  bool HeadlessMessageAnalyzer::canExtract(uint8_t* data, size_t data_size)
  {
    if(data_size < 6)
    {
      return false;
    }

    uint16_t payload_len = (data[4]<<8) | data[5];

    if (data_size < (size_t)payload_len + 6)
    {
      return false;
    }
    
    m_processStatus.counter =(data[0] << 24) | (data[1]<<16) | (data[2]<<8) | data[3];
    m_processStatus.payload_len = payload_len;

    return true;
  }

  void HeadlessMessageAnalyzer::extract(uint8_t* data, size_t data_size, AnalysisResult& analysis_result)
  {
    //printf("extract packet type id: %u %u\n", analysis_result.packet_type_id, m_processStatus.payload_len);
    uint16_t whole_packet_len = m_processStatus.payload_len + 6; // 6 = 2 bytes packet type + 4 bytes counter and length
    memcpy(m_processStatus.msg_buff, m_processStatus.packet_type_check_list, 2);
    memcpy(m_processStatus.msg_buff + 2, data, whole_packet_len - 2);

    //fill analysis_result with DONE, consider refactor as inline function
    analysis_result.status = AnalysisStatus::DONE;
    analysis_result.data = m_processStatus.msg_buff;
    analysis_result.length = whole_packet_len;
    analysis_result.payload_offset = 6;
    analysis_result.payload_len = m_processStatus.payload_len;
    analysis_result.read_size = whole_packet_len - m_processStatus.msg_read_index;

    m_processStatus.flag = 0;
    m_processStatus.msg_read_index = 0;
  }

  void HeadlessMessageAnalyzer::reset()
  {
    m_processStatus.flag = 0;
    m_processStatus.msg_read_index = 0;
  }

  std::string& HeadlessMessageAnalyzer::name()
  {
    return m_format;
  }
}