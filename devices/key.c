#include "device.h"

unsigned char GetKey(void)
{

  int nValue;

  InitADC(0);

  nValue = ReadADC();
  
  // jos mik��n n�pp�in ei ole alhaalla
  // saadaan 1023, mutta siirtym�vaiheessa
  // saattaa tulla hieman pienempi� arvoja
  // tulkitaan nekin ei n�pp�iksi
  if (nValue > 900)
     return NO_KEY;  
  
  // luetaan 10 kertaa ja k�ytet��n keskiarvoa
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
  else return NO_KEY; // ei yht��n alhaalla
}







