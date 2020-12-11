#ifndef CRC8_H_

#ifndef F_CPU
#define F_CPU 16000000L
#endif

#define CRC8_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint8_t crc8( uint8_t* data, uint16_t number_of_bytes_in_data );

#ifdef __cplusplus
}
#endif

#endif


