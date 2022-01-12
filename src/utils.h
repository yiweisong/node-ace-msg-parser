#ifndef BINARY_PACKET_H
#define BINARY_PACKET_H

#include <stdint.h>
#include "types.h"

uint16_t calc_crc(uint8_t* buff, uint32_t nbyte);
int is_nmea_char(char c);
const char* nmea_type(int index);

#endif