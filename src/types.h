#ifndef TYPES_H
#define TYPES_H

#include <vector>
#include <string>

#define USER_PREAMB 0x55
#define NMEA_HEAD 0x24

namespace Aceinna {
    const static std::string FORMAT_NMEA = "NMEA";
    const static std::string FORMAT_ACEINNA_BINARY_V1 = "AceinnaBinaryV1";
    const static std::string FORMAT_ACEINNA_BINARY_V2 = "AceinnaBinaryV2";
    const static std::string FORMAT_ACEINNA_BINARY_V3 = "AceinnaBinaryV3";
    const static std::string FORMAT_ACEINNA_BINARY_V4 = "AceinnaBinaryV4";
    const static std::string FORMAT_ACEINNA_BINARY_UDP = "AceinnaBinaryUdp";

    enum class MessageFormat : uint8_t{
        NMEA,
        ACEINNA_BINARY_V1,
        ACEINNA_BINARY_V2,
        ACEINNA_BINARY_V3
    };

    typedef struct
    {
        // user binary
        uint8_t binary_flag;
        uint8_t binary_msg_header[4]; //preamble
        uint16_t binary_msg_read_index;
        uint16_t binary_payload_len;
        uint8_t binary_wrapper_len;
        uint8_t binary_msg_buff[2048];
        uint16_t binary_packet_type;
        uint8_t binary_packet_len_type;

        uint8_t nmea_flag;
        uint32_t nmea_msg_len;
        uint8_t nmea_msg_buff[256];
    } ParseStatus;

    typedef struct
    {
        uint32_t binary_msg_len;
        uint32_t nmea_msg_len;
    } PacketInfo;

    struct PacketType {
        uint16_t id;
        std::string name;
        std::vector<uint8_t> raw;
    };
}

#endif