/*================================================================================
*     PSAMK      Tekniikka Kuopio
*---------------------------------------------------------------------------------
*
*     Kurssi:     Ohjelmistoprojekti
*
*     aihe:       tietoliikenne ArduinoUno - PC
*
*     Ohjelmoija  MakV‰ 28.2.2012
*
*     k‰‰nt‰j‰:   GCC
*
*     T‰m‰ tiedostossa on sarjaliikenteen perusfunktiot:
*
*     InitPort, GetByte, PutByte
*
*
=============================================================================*/

// character places in buffer
#define COMM_STX        0
#define COMM_LEN_HIGH   1
#define COMM_LEN_LOW    2
#define COMM_HEADER_LEN 3

// this many SYNs before message at least
#define COMM_SYN_CHAR_COUNT 5

#include <avr/io.h>        
#include <stdlib.h>             
#include "protocol.h"
void AddCheckSum( unsigned char *);

/*******************************************************************
**
** Output:   */ void  /*  
**                                 
** Function: */  AddCheckSum( unsigned char *szBuffer )/*  leaving data
**
** Description: calculates the checksum for the leaving data
**              
**             
** Remarks:   
**
********************************************************************/
{ 
    unsigned short iTemp;
    unsigned short cBytes;

    cBytes = 256 * szBuffer[ COMM_LEN_HIGH ] + szBuffer[ COMM_LEN_LOW ];

    for( iTemp= 0; iTemp < cBytes ; iTemp++ )
   {
        szBuffer[ COMM_HEADER_LEN + cBytes ] += szBuffer[ iTemp + COMM_HEADER_LEN ];
   }

}


/*******************************************************************
**
** Output:   */ int   /*
**                                 
** Function: */ SendMsg( unsigned char *pData, /*  leaving data
**           */          unsigned short cBytes )/*  message length
**
** Description: sends message in the envelope to other end
**              
**
** Remarks:   
**
********************************************************************/
{
    unsigned char   szOutput[ COMM_MSG_LEN_MAX ] = { 0 };

    szOutput[ COMM_STX ] = STX;
    szOutput[ COMM_LEN_LOW ] = cBytes % 256;
    szOutput[ COMM_LEN_HIGH ] = cBytes / 256;
    { unsigned char  i;
      for ( i = 0; i < cBytes; i++ ) 
         {
           szOutput[ COMM_HEADER_LEN + i ] = pData[ i ];
      }
    }
    AddCheckSum( szOutput );
    szOutput[ cBytes + COMM_HEADER_LEN + 1 ] = ETX;

    {
      unsigned char i;
      for( i = 0; i <  (cBytes + COMM_HEADER_LEN + 1 + 1); i++)
        PutByte(szOutput[i], 1);
    }

    return 1;
}

/*******************************************************************
**
** Output:   */ int  /*  
**                                 
** Function: */ ReceiveMsg( unsigned char *pData, /*  coming data
**           */             unsigned short *pcBytes )/*  message length
**
** Description: reads message in the envelope from other end
**              
**             
** Remarks:   
**
********************************************************************/
{
   // DWORD cBytesRead = 0;
    unsigned char  chLast;
    unsigned short lenMsg;
    unsigned short cData;
    unsigned char  chkSum;
    int  cSYN = 0;


    // sychronize to the STX byte
    //
    // sometimes the synchronizing can happend at wrong place ( into data bytes )
    // if this is a problem , the protocol should be developed so that
    // we use series of SYN bytes for this purpose
    //
    // all commands have SYN characters before the message
       while ( cSYN < COMM_SYN_CHAR_COUNT )
      {
         chLast= GetByte(1);
         if ( chLast == SYN )
            cSYN++;
      }

    // message starts with STX character
    while ( 1 )   // wait until STX comes
    {
    		chLast = GetByte(1);
         if ( chLast != STX)
             continue;
         else
             break;
    }

    // message len
   GetByte(1); // only short messages used, discard upper byte
   lenMsg = GetByte(1);
   if ( lenMsg > COMM_MSG_LEN_MAX )
      return 0;

    // pick up data
    cData = 0;
    while (  cData < lenMsg )
    {
        chLast = GetByte(1);
        if ( cData < *pcBytes ) // space for all characters ?
           pData[ cData++ ] = chLast;
        else
           return 0;
    }

    // check sum
    chkSum = GetByte(1);

    // etx  is the last character in message
    chLast = GetByte(1);
    if ( chLast != ETX)
      return 0;

    // is check sum right ?
    {
        unsigned  short iTemp;
        unsigned  char dataSum = 0;

        dataSum  = 0;
        for( iTemp= 0; iTemp <   lenMsg; iTemp++)
            dataSum += pData[ iTemp ];

        if ( chkSum - dataSum == 0 )
        {
            *pcBytes = lenMsg; // return message len
            return 1;
        }
        else
           return 0;
    }
}
