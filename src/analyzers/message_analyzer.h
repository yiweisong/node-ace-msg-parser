#ifndef MESSAGE_ANALYZER_H
#define MESSAGE_ANALYZER_H

#include <vector>
#include "../types.h"

namespace Aceinna {
    enum class AnalysisStatus: uint8_t {
        UNKOWN,
        DETERMINING,
        DETERMINED,
        EXTRACTING,
        DONE,
        CRC_ERROR,
        FAILED,
    };

    struct AnalysisResult {
        AnalysisStatus status;
        uint8_t format_id;
        uint16_t packet_type_id;
        uint8_t* data;
        uint16_t length;
        uint16_t read_size;
        uint16_t payload_offset;
        uint16_t payload_len;

        AnalysisResult() : status(AnalysisStatus::DETERMINING), packet_type_id(0), data(nullptr), length(0), read_size(0), payload_offset(0), payload_len(0) {}
    };

    struct NMEAAnalysisProcessStatus{
        uint8_t flag;
        uint32_t read_index;
        uint8_t nmea[256];
        uint16_t packet_type_id;

        NMEAAnalysisProcessStatus() : flag(0), read_index(0), packet_type_id(0) {}
    };

    struct AceinnaBinaryAnalysisProcessStatus{
        uint8_t flag;
        uint8_t format_id;
        uint8_t msg_header[4];
        uint16_t msg_read_index;
        uint16_t payload_len;
        uint8_t msg_buff[2048];
        uint16_t packet_type_id;
        uint8_t packet_len_type;
        uint16_t counter;
        uint8_t wrapper_len;
        uint8_t payload_offset;

        AceinnaBinaryAnalysisProcessStatus() : flag(0), format_id(0), msg_read_index(0), payload_len(0), packet_type_id(0), packet_len_type(0), counter(0), wrapper_len(0), payload_offset(0) {}
    };

    class MessageAnalyzer {
    public:
        MessageAnalyzer() = default;
        virtual ~MessageAnalyzer() = default;
        virtual void determine(uint8_t byte_data, AnalysisResult& analysis_result) = 0;
        virtual bool canExtract(uint8_t* data, size_t data_size) = 0;
        virtual void extract(uint8_t* data, size_t data_size, AnalysisResult& analysis_result) = 0;
        virtual void reset() = 0;
        virtual std::string &name() = 0;
    };

    class NMEAMessageAnalyzer : public MessageAnalyzer {
    public:
        NMEAMessageAnalyzer(std::vector<PacketType> allow_packet_types);
        ~NMEAMessageAnalyzer() = default;
        void determine(uint8_t byte_data, AnalysisResult& analysis_result) override;
        bool canExtract(uint8_t* data, size_t data_size) override;
        void extract(uint8_t* data, size_t data_size, AnalysisResult& analysis_result) override;
        void reset() override;
        std::string &name() override;
    private:
        inline bool isNMEAChar(char c);
        inline bool checkPacketType(uint8_t* nmea, std::vector<uint8_t> raw_packet_type, size_t size);
    private:
        std::string m_format;
        std::vector<PacketType> m_allowPacketTypes;
        NMEAAnalysisProcessStatus m_processStatus;
    };

    class AceinnaBinaryMessageAnalyzer : public MessageAnalyzer {
    public:
        AceinnaBinaryMessageAnalyzer(std::string format, std::vector<PacketType> allow_packet_types, bool skip_crc);
        ~AceinnaBinaryMessageAnalyzer() = default;
        void determine(uint8_t byte_data, AnalysisResult& analysis_result) override;
        bool canExtract(uint8_t* data, size_t data_size) override;
        void extract(uint8_t* data, size_t data_size, AnalysisResult& analysis_result) override;
        void reset() override;
        std::string &name() override;
    private:
        bool m_skipCRC;
        std::string m_format;
        std::vector<uint16_t> m_allowPacketTypeIds;
        AceinnaBinaryAnalysisProcessStatus m_processStatus;
    };
}

#endif