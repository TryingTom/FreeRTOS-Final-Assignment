/*================================================================================
*     SAVONIA      Tekniikka Kuopio
*---------------------------------------------------------------------------------
*
*     T�m� tiedostossa on l�mp�mittauksen perusfunktiot.
*
*     Anturin pinni m��ritelty onewire.h :ssa
*
*
=============================================================================*/
//#include <avr/version.h>
#include <avr/io.h>
//#include <avr/pgmspace.h>
#include <avr/interrupt.h>
//#include <avr/eeprom.h>
#include <util/delay.h>
//#include <string.h>
//#include <stdint.h>
#include <stdio.h>
//#include <stdlib.h>

//#include "onewire.h"

void    USART_Init(unsigned char jakaja);
int     USART_Receive( FILE *in );
int     USART_Transmit(  char merkki, FILE *in );

unsigned char GetSensorCount(void);
long    GetTemperature( unsigned nSensor);


//-----------------------------------------------------------------------------//
//**************************  P � � O H J E L M A  ****************************//
//-----------------------------------------------------------------------------//



int main()
{
	long           temperature;
	unsigned char  numberOfSensors;


	USART_Init(207); // 207 = 4800 bd 103 = 9600 bd 51 = 19200 bd ja  8 databitti�, 2stop bitti�

	fdevopen(USART_Transmit, USART_Receive); // mahdollistaa printf - k�yt�n
	
	sei();
	numberOfSensors = GetSensorCount();

	printf("L�ytyi %i anturia\n\r", numberOfSensors );

    while(1) 
    { // luuppi
	
	 if (numberOfSensors)		
	 {
          temperature  = GetTemperature(0);
		  printf("\n\rlampo = %li.%i C", temperature / 10000, (temperature % 10000) / 1000);
     }
       _delay_ms(5000); // huili
					   
    }				 
} 

	         
				 
int USART_Receive( FILE *in )
{
  /* odotetaan tulevaa merkki� */
  while ( !(UCSR0A & (1<<RXC0)) );

  /* poimitaan merkki dataportista */
  return (int)UDR0;
}

int USART_Transmit(  char merkki, FILE *in )
{
  /* Odotetaan, ett� l�hetyspuoli tyhj� */
  while ( !( UCSR0A & (1<<UDRE0)) );

  /* Laitetaan l�htev� merkki datapuskuriin => l�hetys alkaa */
  UDR0 = merkki;
  return 0; // ok
}
void USART_Init(unsigned char jakaja)
{
  /* asetetaan nopeus */
  UBRR0H = 0;
  UBRR0L = (unsigned char) jakaja;

  UCSR0A = (1<<U2X0);

  /* Vastaanotin ja l�hetin p��lle */
  UCSR0B = (1<<RXEN0)|(1<<TXEN0);

  /* merkin parametrit: 8 databitti�, 2stop bitti� */
  UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

         


      
 
