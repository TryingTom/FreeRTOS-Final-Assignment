#include <avr\io.h>              // Most basic include files
#include "adc.h"
// oletus kellotaajuus 16 Mhz Atmega328P
int ReadADC(void)
{
  char i;
  int  lukema;

  int  tulos = 0;

  ADCSRA |= 1 << ADEN; // laite p��lle

  //pitk�kestoinen lukema pois
  ADCSRA |= (1<<ADSC); //
  while(!(ADCSRA & (1 << ADIF))); // ADIF tulee p��lle lopuksi

  // do the ADC conversion 8 times for better accuracy
  for(i=0;i<8;i++)
  {
    ADCSRA |= (1<<ADSC); // aloita muunnos
    while(!(ADCSRA & (1 << ADIF))); // ADIF tulee p��lle lopuksi

    lukema = ADCL;         // alaosa luetaan ensiksi
    lukema += (ADCH << 8); // yl�osan luenta

    // otetaan 8 lukemaa summaksi
    tulos += lukema;
  }

  tulos = tulos >> 3; // lukemien keskiarvo

  ADCSRA &= ~(1<<ADEN); // AD-muunnin pois p��lt�
  return tulos;
}

void InitADC(char portti)
{
  // sis�inen referenssij�nnite + kanavan valinta
  //ADMUX = (1 << REFS1) | (1 << REFS0) | portti;
  ADMUX =  (1 << REFS0) | portti;

  // laite p��lle + kellotaajuudeksi 16MHz / 128 = 125kHz
  ADCSRA = (1<<ADEN) | (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2);

  ReadADC(); // nollataan muunnin lukemalla kertaalleen
}


