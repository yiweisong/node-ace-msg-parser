#ifndef TYPES_H
#define TYPES_H

#define USER_PREAMB 0x55
#define NEAM_HEAD 0x24

#define MAX_NMEA_TYPES 17

typedef struct
{
    uint8_t nmea_flag;
    uint8_t flag;
    uint8_t header_len;
    uint8_t header[4];
    uint32_t nbyte;
    uint8_t buff[256];
    uint32_t nmeabyte;
    uint8_t nmea[256];
    uint8_t ntype;
} usrRaw;

typedef struct
{
    char* packetType;
    uint8_t packetTypeLen;
    uint8_t* payload;
    uint8_t payloadLen;
} UartBinaryPacket;

typedef struct
{
    uint8_t payload;
} ethernetBinaryPacket;

#endif