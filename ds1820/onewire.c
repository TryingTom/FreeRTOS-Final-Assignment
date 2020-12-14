#ifndef F_CPU
#define F_CPU 16000000L
#endif
#include <avr/io.h>
#include <util/delay.h>
#include <util/atomic.h>

#include "onewire.h"

uint8_t getSensorIDs[MAXSENSORS][OW_ROMCODE_SIZE];



#define OW_GET_IN()   ( OW_IN & (1<<OW_PIN))
#define OW_OUT_LOW()  ( OW_OUT &= (~(1 << OW_PIN)) )
#define OW_OUT_HIGH() ( OW_OUT |= (1 << OW_PIN) )
#define OW_DIR_IN()   ( OW_DDR &= (~(1 << OW_PIN )) )
#define OW_DIR_OUT()  ( OW_DDR |= (1 << OW_PIN) )

uint8_t ow_input_pin_state()
{
	return OW_GET_IN();
}

void ow_parasite_enable(void)
{
	OW_OUT_HIGH();
	OW_DIR_OUT();
}

void ow_parasite_disable(void)
{
	OW_DIR_IN();
}



//********************************************************//
//   Funktion tarkoitus: V‰yl‰ t‰ytyy vet‰‰ aina ensin    // 
//       alatilaan liikennˆinin aloittamiseksi            //
//********************************************************//

uint8_t ow_reset(void)
{
	uint8_t err;
	
	OW_OUT_LOW();
	OW_DIR_OUT();            // Vedet‰‰n OW-Pinni alas: 480us
	_delay_us(480);

    cli();
	// Asetetaan pinni sis‰‰ntuloksi - odotetaan, ett‰ orjat vet‰v‰t alas
	OW_DIR_IN(); // sis‰‰ntulo
	
	_delay_us(64);       // was 66
	err = OW_GET_IN();   // ei laitteita havaittu
	                     // jos err!=0: kukaan ei vet‰nyt alas, s‰ilyy yl‰tilassa
	
	// Viiveen j‰lkeen orjien tulisi vapauttaa linja
	// ja sis‰‰ntulo-pinni palaa takaisin yl‰tilaan ylˆsvetovastuksen toimesta
	
	_delay_us(480 - 64);       // was 480-66
	if( OW_GET_IN() == 0 ) {
		err = 1;             // short circuit, expected low but got high
	}
	
	sei();
	return err;
}


/* Timing issue when using runtime-bus-selection (!OW_ONE_BUS):
   The master should sample at the end of the 15-slot after initiating
   the read-time-slot. The variable bus-settings need more
   cycles than the constant ones so the delays had to be shortened 
   to achive a 15uS overall delay 
   Setting/clearing a bit in I/O Register needs 1 cyle in OW_ONE_BUS
   but around 14 cyles in configureable bus (us-Delay is 4 cyles per uS) */
static uint8_t ow_bit_io_intern( uint8_t b, uint8_t with_parasite_enable )
{
        cli();
        {
		OW_DIR_OUT();    // drive bus low
		_delay_us(2);    // T_INT > 1usec accoding to timing-diagramm
		if ( b ) {
			OW_DIR_IN(); // to write "1" release bus, resistor pulls high
		}

		// "Output data from the DS18B20 is valid for 15usec after the falling
		// edge that initiated the read time slot. Therefore, the master must 
		// release the bus and then sample the bus state within 15ussec from 
		// the start of the slot."
		_delay_us(15-2-OW_CONF_DELAYOFFSET);
		
		if( OW_GET_IN() == 0 ) {
			b = 0;  // sample at end of read-timeslot
		}
	
		_delay_us(60-15-2+OW_CONF_DELAYOFFSET);

		OW_DIR_IN();
	
		if ( with_parasite_enable ) {
			ow_parasite_enable();
		}
	
	}  

	_delay_us(OW_RECOVERY_TIME); // may be increased for longer wires
	sei();

	return b;
}

uint8_t ow_bit_io( uint8_t b )
{
	return ow_bit_io_intern( b & 1, 0 );
}

uint8_t ow_byte_wr( uint8_t b )
{
	uint8_t i = 8, j;
	
	do {
		j = ow_bit_io( b & 1 );
		b >>= 1;
		if( j ) {
			b |= 0x80;
		}
	} while( --i );
	
	return b;
}

uint8_t ow_byte_wr_with_parasite_enable( uint8_t b )
{
	uint8_t i = 8, j;
	
	do {
		if ( i != 1 ) {
			j = ow_bit_io_intern( b & 1, 0 );
		} else {
			j = ow_bit_io_intern( b & 1, 1 );
		}
		b >>= 1;
		if( j ) {
			b |= 0x80;
		}
	} while( --i );
	
	return b;
}


uint8_t ow_byte_rd( void )
{
	// read by sending only "1"s, so bus gets released
	// after the init low-pulse in every slot
	return ow_byte_wr( 0xFF ); 
}


uint8_t ow_rom_search( uint8_t diff, uint8_t *id )
{
	uint8_t i, j, next_diff;
	uint8_t b;
	
	if( ow_reset() ) {
		return OW_PRESENCE_ERR;         // error, no device found <--- early exit!
	}
	
	ow_byte_wr( OW_SEARCH_ROM );        // ROM search komento
	next_diff = OW_LAST_DEVICE;         // unchanged on last device
	
	i = OW_ROMCODE_SIZE * 8;            // 8 tavua
	
	do {
		j = 8;                          // 8 bitti‰
		do {
			b = ow_bit_io( 1 );         // lue bitti
			if( ow_bit_io( 1 ) ) {      // lue komplementti bitti
				if( b ) {               // 0b11
					return OW_DATA_ERR; // data error <--- early exit!
				}
			}
			else {
				if( !b ) {              // 0b00 = 2 devices
					if( diff > i || ((*id & 1) && diff != i) ) {
						b = 1;          // nyt 1
						next_diff = i;  // next pass 0
					}
				}
			}
			ow_bit_io( b );             // kirjoita bitti
			*id >>= 1;
			if( b ) {
				*id |= 0x80;            // talleta bitti
			}
			
			i--;
			
		} while( --j );
		
		id++;                           // seuraava tavu
	
	} while( i );
	
	return next_diff;                   // jatketaan etsint‰‰
}


static void ow_command_intern( uint8_t command, uint8_t *id, uint8_t with_parasite_enable )
{
	uint8_t i;

	ow_reset();

	if( id ) {
		ow_byte_wr( OW_MATCH_ROM );     // yksitt‰iselle laitteelle, osoitetaan tietty‰ anturia
		i = OW_ROMCODE_SIZE;			
		do {
			ow_byte_wr( *id );
			id++;
		} while( --i );
	} 
	else {
		ow_byte_wr( OW_SKIP_ROM );      // kaikille laitteille, s‰‰stet‰‰n aikaa v‰yl‰ll‰, 
										// jossa vain yksi anturi
	}									 
	
	if ( with_parasite_enable  ) {
		ow_byte_wr_with_parasite_enable( command );
	} else {
		ow_byte_wr( command );
	}
}

void ow_command( uint8_t command, uint8_t *id )
{
	ow_command_intern( command, id, 0);
}

void ow_command_with_parasite_enable( uint8_t command, uint8_t *id )
{
	ow_command_intern( command, id, 1 );
}

uint8_t search_sensors(void)
{
	uint8_t i;
	uint8_t id[OW_ROMCODE_SIZE];
	uint8_t diff, numberOfSensors;
	
	_delay_ms(1500*3);

	ow_reset();

	numberOfSensors = 0;
	
	diff = OW_SEARCH_FIRST;
	while ( diff != OW_LAST_DEVICE && numberOfSensors < MAXSENSORS ) {
		DS18X20_find_sensor( &diff, &id[0] );
	
		if (diff != OW_PRESENCE_ERR)
		{
		  for ( i=0; i < OW_ROMCODE_SIZE; i++ )
		    	getSensorIDs[numberOfSensors][i] = id[i];
	  	
		  numberOfSensors++;
		}
		else
		  break;
	}
	
	return numberOfSensors;
}

unsigned char GetSensorCount(void)
{
  return search_sensors();
}

long    GetTemperature( unsigned nSensor)
{ long temperature;
	
   DS18X20_start_meas( DS18X20_POWER_EXTERN, NULL);
	  
   _delay_ms( DS18B20_TCONV_12BIT );

   DS18X20_read_maxres( &getSensorIDs[nSensor][0], &temperature );
   return temperature;
}
