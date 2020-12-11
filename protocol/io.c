#ifndef F_CPU
#define F_CPU 16000000L
#endif
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h> 
#include <string.h>
#include <stdlib.h>
 /*
   FreeRTOS on Arduino Uno  

*/

/// Scheduler include files. --------------------------------------------
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
#include "semphr.h"
#include "serial.h"

extern xComPortHandle xSerialPort;

void PutByte( unsigned char ch, unsigned nComPort )
{
    nComPort= 0; // disables warning
    xSerialPutChar(&xSerialPort, ch); // kirjoitetaan merkki PC:lle
}
unsigned char GetByte(unsigned nComPort)
{
    static  UBaseType_t ch;
   (void)nComPort; // disable warning
   
    while( xSerialGetChar( &xSerialPort, &ch)== pdFAIL); // odotetaan kunnes saadaan merkki
	 
   return (unsigned char)ch; 
}
