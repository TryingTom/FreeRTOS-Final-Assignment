#include "device.h"

unsigned char GetKey(void)
{

  int nValue;

  InitADC(0);

  nValue = ReadADC();
  
  // jos mikään näppäin ei ole alhaalla
  // saadaan 1023, mutta siirtymävaiheessa
  // saattaa tulla hieman pienempiä arvoja
  // tulkitaan nekin ei näppäiksi
  if (nValue > 900)
     return NO_KEY;  
  
  // luetaan 10 kertaa ja käytetään keskiarvoa
  // transienttilukemien eliminoimiseksi
  nValue = 0;
  for(int i=0; i < 10; i++)
   nValue += ReadADC();
  nValue /= 10; // keskiarvo

  // tunnistetaan
  if (nValue < 50)
	  return IDK_RIGHT;
  else if (nValue < 230)
	  return IDK_UP;
  else if (nValue < 450)
	  return IDK_DOWN;
  else if (nValue < 650)
	  return IDK_LEFT;
  else if (nValue < 900)
	  return IDK_SELECT;
  else return NO_KEY; // ei yhtään alhaalla
}







