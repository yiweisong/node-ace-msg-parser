#ifndef TYPES_H
#define TYPES_H

#define USER_PREAMB 0x55
#define NEAM_HEAD 0x24

#define MAX_NMEA_TYPES 17


typedef struct
{
    // user binary
    uint8_t binary_flag;
    uint8_t binary_msg_header[4];
    uint8_t binary_msg_header_len;
    uint32_t binary_msg_len;
    uint8_t binary_msg_buff[2048];
    uint16_t binary_packet_type;
    uint8_t binary_packet_len_type;

    uint8_t nmea_flag;
    uint32_t nmea_msg_len;
    uint8_t nmea_msg_buff[256];
} ParseStatus;

typedef struct
{
    char* packet_type;
    uint8_t payload_len;
} PacketInfo;


#endif