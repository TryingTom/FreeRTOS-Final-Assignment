/*************************************************************PSAMK *
**
** File: protocol.h
**
** Customer: none
**
** Project: Demo
**
** Status:   [ ] Under development   
**           [x] Proto version
**           [ ] Ready                     28.2.2012  
**
** Compiler: GCC
**
** Author:   MakVä / Savonia, Kuopio, Finland
**
** Description:  Specifies PC/ArduinoUno  - communication interface
**
**                                                                           
************************************************************************/
//
// it's very important to use this very same header file 
// at both ends of the data transmission
//
// **** INCREASE THE FOLLOWING NUMBER ALLWAYS WHEN YOU EDIT THIS FILE ****

#define COMM_VERSION 7

//
// the line parameters are defined here
//
#define COMM_BAUDRATE 38400  // use only standard values 1200, 2400,...
#define COMM_PARITY_USED 0   // 1 = USED 0 = NOT USED
#define COMM_PARITY_ODD  0   // 1 = odd parity  0 = dont't care
#define COMM_PARITY_EVEN 0   // 1 = even parity 0 = don't care
#define COMM_STOPBITS    2   // 1 15 2
#define COMM_BYTEWIDTH   8   // 5,6,7,8

#define COMM_TIMEOUT 300 // time to complete VREX call at remote end in ms
//
// Check the data field lengths that they are same that in comment text
//
typedef unsigned char MESSAGEID;         // one char
#define MESSAGE_IDENTIFICATION_BYTE   0  // messages are identified with a constant

//
// character codes used in protocol
//
#define STX   2
#define ETX   3
#define SYN  22

//
// the next constant specifies the trial count to send same message
// when responsed with nack or responsed not at all
//
#define COMM_TRY_MAX 3
#define COMM_MSG_LEN_MAX 10 
//
// THE MESSAGE FORMAT
//
//  [STX LEN DATA CHS ETX]
//
//  STX, CHS, ETX = 1 byte each LEN = 2 bytes
//  LEN specifies the length of the data field as bytes
//  CHS is sum of data bytes 
//
// THE PROTOCOL
//
// Windows/NT is allways the active side of  the data traffic. The messages from Win/NT
// are acknowledged either with ACK/NACK message or if the response contains data with
// the command specific reply message.
//
// Win/NT send the same message COMM_TRY_MAX times or until not NACK - responsed.
// The reason for the NACK - response can be either the corruption of data in the
// message or illegal parameter values. The last situation is supposed to happend 
// only in the testing phase.
//


//
// the general message buffer
//
typedef struct
{
	char  header[ 3 ];
	char  datax[ COMM_MSG_LEN_MAX ];
	char  footer[ 2 ];
}
MSG_BUFFER;

#define SET_PORT    1
#define GET_VALUE   2
#define COMM_NACK  99
#define COMM_ACK   98

//
//  functions return the information of success
//
#define COMM_OK 1
#define COMM_FAILED 0

extern int  ReceiveMsg( unsigned char *pData, unsigned short *pcBytes);
extern int  SendMsg( unsigned char *pData, unsigned short cBytes );
extern void PutByte( unsigned char ch, unsigned nComPort );
extern  unsigned char GetByte(unsigned nComPort);

