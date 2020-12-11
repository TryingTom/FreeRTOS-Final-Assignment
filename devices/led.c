//
// tämä versio ArduinoAtmega 2560 24.3.2017 ktmakva
//

#include <avr/io.h>
#include "led.h"
#define SetBit(port,number) { port |= (1 << number); }
#define ClearBit(port,number) { port &= ~(1 << number); }

void LedOn( char number )
{
    if(number == LED_MOTHER_BOARD)
    {
      SetBit(DDRB, 7 );  // lähtö
      SetBit(PORTB,7);
    }
    // tähän muut ledit kaikki erikseen
}
void LedOff(char number)
{
  if(number == LED_MOTHER_BOARD)
  {
     SetBit(DDRB, 7 );  // lähtö
    ClearBit(PORTB,7);
  }
  // tähän muut ledit kaikki erikseen
}






