#include "utils.h"

uint16_t calc_crc(uint8_t* buff, uint32_t nbyte) {
	uint16_t crc = 0x1D0F;
	uint32_t i, j;
	for (i = 0; i < nbyte; i++) {
		crc = crc ^ (buff[i] << 8);
		for (j = 0; j < 8u; j++) {
			if (crc & 0x8000) {
				crc = (crc << 1) ^ 0x1021;
			}
			else {
				crc = crc << 1;
			}
		}
	}
	crc = crc & 0xffff;
	return crc;
}

int is_nmea_char(char c)
{
	if (c >= 'A' && c <= 'Z') {
		return 1;
	}
	else if (c >= '0' && c <= '9') {
		return 1;
	}
	else if (c == '$' ,c == '-' || c == ',' || c == '.' || c == '*') {
		return 1;
	}
	return 0;
}

const char* NMEATypeList[MAX_NMEA_TYPES] = { "$GPGGA","$GNGGA", "$GPRMC", "$GNRMC", "$GPGSV", "$GLGSV", "$GAGSV", "$BDGSV", "$GPGSA", "$GLGSA", "$GAGSA", "$BDGSA", "$GPZDA", "$GNZDA", "$GPVTG", "$PASHR", "$GNINS" };

const char * nmea_type(int index)
{
	return NMEATypeList[index];
}