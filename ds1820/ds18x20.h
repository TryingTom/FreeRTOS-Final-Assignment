#ifndef DS18X20_H_
#define DS18X20_H_

#ifndef F_CPU
#define F_CPU 16000000L
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdint.h>

// DS18x20 EERPROM support disabled(0) or enabled(1) :
#define DS18X20_EEPROMSUPPORT     1
// max. resolution functions disabled(0) or enabled(1):
#define DS18X20_MAX_RESOLUTION    1
// extended output via UART disabled(0) or enabled(1) :
#define DS18X20_VERBOSE           1


/* palautettavat arvot */
#define DS18X20_OK                0x00
#define DS18X20_ERROR             0x01
#define DS18X20_START_FAIL        0x02
#define DS18X20_ERROR_CRC         0x03

#define DS18X20_INVALID_DECICELSIUS  2000

#define DS18X20_POWER_PARASITE    0x00
#define DS18X20_POWER_EXTERN      0x01

#define DS18X20_CONVERSION_DONE   0x00
#define DS18X20_CONVERTING        0x01

/* DS18X20 m��ritykset (datasheetist�) */
#define DS18S20_FAMILY_CODE       0x10
#define DS18B20_FAMILY_CODE       0x28
#define DS1822_FAMILY_CODE        0x22

#define DS18X20_CONVERT_T         0x44
#define DS18X20_READ              0xBE
#define DS18X20_WRITE             0x4E
#define DS18X20_EE_WRITE          0x48
#define DS18X20_EE_RECALL         0xB8
#define DS18X20_READ_POWER_SUPPLY 0xB4

/*
#define DS18B20_CONF_REG          4
#define DS18B20_9_BIT             0
#define DS18B20_10_BIT            (1<<5)
#define DS18B20_11_BIT            (1<<6)
#define DS18B20_12_BIT            ((1<<6)|(1<<5))
#define DS18B20_RES_MASK          ((1<<6)|(1<<5))

// undefined bits in LSB if 18B20 != 12bit
#define DS18B20_9_BIT_UNDF        ((1<<0)|(1<<1)|(1<<2))
#define DS18B20_10_BIT_UNDF       ((1<<0)|(1<<1))
#define DS18B20_11_BIT_UNDF       ((1<<0))
#define DS18B20_12_BIT_UNDF       0

// conversion times in milliseconds
#define DS18B20_TCONV_11BIT       DS18B20_TCONV_12_BIT/2
#define DS18B20_TCONV_10BIT       DS18B20_TCONV_12_BIT/4
#define DS18B20_TCONV_9BIT        DS18B20_TCONV_12_BIT/8
*/

#define DS18B20_TCONV_12BIT       750
#define DS18S20_TCONV             DS18B20_TCONV_12_BIT

// constant to convert the fraction bits to cel*(10^-4)
#define DS18X20_FRACCONV          625

// scratchpad size in bytes
#define DS18X20_SP_SIZE           9

// DS18X20 EEPROM-Support
#define DS18X20_WRITE_SCRATCHPAD  0x4E
#define DS18X20_COPY_SCRATCHPAD   0x48
#define DS18X20_RECALL_E2         0xB8
#define DS18X20_COPYSP_DELAY      10 /* ms */
#define DS18X20_TH_REG            2
#define DS18X20_TL_REG            3

#define DS18X20_DECIMAL_CHAR      '.'


extern uint8_t DS18X20_find_sensor(uint8_t *diff, 
	uint8_t id[]);
extern uint8_t DS18X20_get_power_status(uint8_t id[]);
extern uint8_t DS18X20_start_meas( uint8_t with_external,
	uint8_t id[]);
// returns 1 if conversion is in progress, 0 if finished
// not available when parasite powered
extern uint8_t DS18X20_conversion_in_progress(void);


#if DS18X20_MAX_RESOLUTION
// temperature unit for max. resolution is �C * 10e-4
// examples: -250625 -> -25.0625�C, 1250000 -> 125.0000 �C
extern uint8_t DS18X20_read_maxres( uint8_t id[],
	long *temperaturevalue );
extern uint8_t DS18X20_read_maxres_single( uint8_t familycode,
	int32_t *temperaturevalue );
extern uint8_t DS18X20_format_from_maxres( long temperaturevalue, 
	char s[]);
#endif /* DS18X20_MAX_RESOLUTION */


#if DS18X20_EEPROMSUPPORT
// kirjoita th, tl ja config-rekisteri to scratchpadiin (config ignored on DS18S20)
uint8_t DS18X20_write_scratchpad( uint8_t id[], 
	uint8_t th, uint8_t tl, uint8_t conf);
// lue scratchpad taulukkoon SP
uint8_t DS18X20_read_scratchpad( uint8_t id[], uint8_t sp[], uint8_t n);
// kopioi arvot scratchpadista DS18x20:n eepromiin
uint8_t DS18X20_scratchpad_to_eeprom( uint8_t with_power_extern,
	uint8_t id[] );
// kopioi arvot DS18x20:n eepromilta scratchpadiin
uint8_t DS18X20_eeprom_to_scratchpad( uint8_t id[] );
#endif /* DS18X20_EEPROMSUPPORT */


#if DS18X20_VERBOSE
extern void DS18X20_show_id_uart( uint8_t *id, size_t n );
extern uint8_t DS18X20_read_meas_all_verbose( void );
#endif /* DS18X20_VERBOSE */


#ifdef __cplusplus
}
#endif

#endif
