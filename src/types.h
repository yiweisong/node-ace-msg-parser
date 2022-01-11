#ifndef TYPES_H
#define TYPES_H

#define USER_PREAMB 0x55
#define NEAM_HEAD 0x24

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
    uint8_t payload;
} uartBinaryPacket;

typedef struct
{
    uint8_t payload;
} ethernetBinaryPacket;

#endif