//--------------------------------------------------------------
//	Pohjana käytetty:
//  Realiaikajärjestelmät      syksy 2017   ktMakva 3.10.2017  FreeRTOS on Arduino Atmega 2560
//
//    Tarvikkeet: Arduino + LCD + DS1820
//
//	Työn tehnyt Tomi Heikkinen, ESE17SP. Työ aloitettiin 11.12.2020
//	Työ tehty oppimistarkoitukseen.



#define F_CPU 16000000L

// Scheduler include files. --------------------------------------------
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
#include "semphr.h"
#include "serial.h"
#include "queue.h"
#include "event_groups.h"
// avr tarpeen ajastinpiirin takia
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// c kielen funktiot
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
// paikalliset laitteet
#include "../ds1820/sensor.h"   // lämpömittauksen funktiot
#include "..\devices\device.h"  // lcd-paneeli

// viestien nimet-------------------- (IDM = identification of message)
#define IDM_LEDS 1
#define IDM_UPDATE_DISPLAY 2
#define IDM_DISPLAY_MINMAX 3
#define IDM_DISPLAY_HISTORY 4
#define IDM_DISPLAY_TIME 5
// viestien rakenne
typedef struct
{
	char idMessage; // viestin tunniste
	char data;      // dataa
}DISPLAY_MESSAGE;


// -- taskien esittelyt ---------------
static void vLcdHandler( void *pvParameters );
static void vKeyPadHandler( void *pvParameters );
static void vDoMeasurements( void *pvParameters );

/* Baud rate used by the serial port tasks. */
#define mainCOM_BAUD_RATE			(9600)
#define comBUFFER_LEN				(50)

//-- taskien yhteinen data -------------------------------
#define INTS_MAX 5
// yhteisille tiedoille käytettävät nimet  (IDD = identification of data)
#define IDD_BUTTON 3
#define IDD_LASTKEY 2

int ints[INTS_MAX] = {0};// tietokanta = yhteiset kokonaislukutiedot
//--------------------------------------------------------
//--postilaatikot
static QueueHandle_t xDisplay; // näytön postilaatikko
//--semaforit
static SemaphoreHandle_t xADC; // AD-muuntimen varaussemafori
static SemaphoreHandle_t xDisplaySemaphore; // AD-muuntimen varaussemafori

xComPortHandle xSerialPort;

// kellon kutsut:

// -- taskien esittelyt ---------------
static void vClock( void *pvParameters );
static void vTerminal( void *pvParameters );

/* Baud rate used by the serial port tasks. */
#define mainCOM_BAUD_RATE			(9600)
#define comBUFFER_LEN				(50)

//-- taskien yhteinen data -------------------------------
unsigned  secondsFromMidNight = 0; // tämä
#define INTS_MAX_TIME 6
// yhteisille tiedoille käytettävät nimet  (IDD = identification of data)
#define IDD_HOUR    0
#define IDD_MINUTES 1
#define IDD_SECONDS 2
#define IDD_DAY     3
#define IDD_MONTH   4
#define IDD_YEAR    5
int intsTime[INTS_MAX_TIME] = {0};// tietokanta = yhteiset kokonaislukutiedot
//--------------------------------------------------------
//--postilaatikot
//--semaforit
static SemaphoreHandle_t xClock; // kellokeskeytyksen semafori
//--------------------------------------------------------
// tavallisten funktioiden esittelyt
void StartTimer( int ticks);
void ShowTime(void);
void OwnGets(char *pText);

//--------------------------------------------------------
// pääohjelma luo taskit ja käynnistää systeemin
int main( void )
{
	xDisplay = xQueueCreate( 2, sizeof(DISPLAY_MESSAGE));// luodaan postilaatikko näyttötaskille

	vSemaphoreCreateBinary( xADC );// luodaan semafori
	vSemaphoreCreateBinary( xDisplaySemaphore);

	// alustetaan sarjaportti
    xSerialPort = xSerialPortInitMinimal(0, mainCOM_BAUD_RATE, comBUFFER_LEN ,10);
	lcd_init(LCD_DISP_ON);  // lcd-kuntoon

	// luodaan taskit
	xTaskCreate( vLcdHandler, 0, configMINIMAL_STACK_SIZE, 0,  (tskIDLE_PRIORITY + 2), NULL );
	xTaskCreate( vDoMeasurements, 0, configMINIMAL_STACK_SIZE, 0,  (tskIDLE_PRIORITY + 2), NULL );
	xTaskCreate( vKeyPadHandler, 0, configMINIMAL_STACK_SIZE, 0,  (tskIDLE_PRIORITY + 2), NULL );
	
	xTaskCreate( vClock, 0, configMINIMAL_STACK_SIZE, 0,  (tskIDLE_PRIORITY + 2), NULL );
	xTaskCreate( vTerminal, 0, configMINIMAL_STACK_SIZE, 0,  (tskIDLE_PRIORITY + 2), NULL );
	
	vTaskStartScheduler();  // ajastus päälle

	return 0;
}

static void vDoMeasurements( void *pvParameters )
{
	( void ) pvParameters; // Just to stop compiler warnings.
	int value ;

		static unsigned char  numberOfSensors;
		
		numberOfSensors = GetSensorCount(); // samassa johtimessa lämpöantureiden määrä


		for( ;; )
		{

			if (numberOfSensors)
			{
				value  = (int)(GetTemperature(0)/1000);
		        taskENTER_CRITICAL(); //////////////////////////
		        ints[ IDD_BUTTON ] = value ;                ////
		        taskEXIT_CRITICAL(); ///////////////////////////
			}

			vTaskDelay(100);
		}
	
}
//--------------------- lcd-näyttöä päivittävä taski -------------------------------
static void vLcdHandler( void *pvParameters )
{

	char *pDisplay[] = {"VALUE=%i03   \nBUTTON=%i02"};
	volatile  char *pChDisplay =0,  // osoitin, jota osoittaa nytn pohjatekstiss olevaan kirjaimeen
	*pChVariable=0; // osoitin, jota kytetään muuttujan arvon tulostukseen
	int             i;
	char            szVariable[8]; // muuttujan arvo tulostetaan tähän
	static DISPLAY_MESSAGE message;

	( void ) pvParameters;	/* Just to stop compiler warnings. */


	// taskilla tulee aina  olla ikisilmukka
	for( ;; )//.................
	{
		xQueueReceive(xDisplay,&message, portMAX_DELAY); // odotetaan viestiä
	
		switch( message.idMessage)
		{
			case IDM_UPDATE_DISPLAY:
			xSemaphoreTake( xDisplaySemaphore, portMAX_DELAY ); // ei tarpeen, koska portissa B ei muita laitteita
			if( message.data == 0 )
			pChDisplay = pDisplay[0];
			
			*pDisplay = "Update=%i03 C  \nDisplay=%i02";
			
			// ja vain tämä taski käyttää lcd:tä
			lcd_gotoxy(0,0); // lcd-näytön alkuun
			//// tulostetaan näyttö tässä silmukassa
			while(*pChDisplay != 0)
			{
				// onko muuttujan tulostuspaikka?
				if( *pChDisplay == '%')
				{
					pChDisplay++; //ohitetaan %-merkki
					// minkä tyypin dataa?
					switch(*pChDisplay)
					{
						// int-tyypin kokonaislukuja
						// muuttujan arvo poimitaan ints-taulukosta indeksin määräämästä paikasta
						case 'i': pChDisplay++;
						// indeksi annettu muodossa 09, 10,11,..
						i = (*pChDisplay - '0')*10;
						pChDisplay++;
						i +=  (*pChDisplay- '0'); // indeksi ints-taulukkoon
						taskENTER_CRITICAL(); //////////////////////////////////
						itoa(ints[i],szVariable,10);                        //// // itoa transfers int to string inside the szVariable array
						taskEXIT_CRITICAL();  //////////////////////////////////
						szVariable[3] = szVariable[2];
						szVariable[2] = ',';
						break;
					}
					// tulostetaan muuttuja
					pChVariable = szVariable;
					while(*pChVariable != 0)
					{
						lcd_putc(*pChVariable); // merkki näkyviin
						pChVariable++; // seuraava kirjain
					}
				}
				else if(*pChDisplay == '\n')
				{
					lcd_gotoxy(0,1);
				}
				else
				// näytön pohjateksti
				lcd_putc(*pChDisplay); // merkki näkyviin

				pChDisplay++; // seuraava kirjain
			}
			xSemaphoreGive( xDisplaySemaphore); // Vapautetaan semaphori
			break;
			
			case IDM_DISPLAY_HISTORY:
			xSemaphoreTake( xDisplaySemaphore, portMAX_DELAY ); // ei tarpeen, koska portissa B ei muita laitteita
			if( message.data == 0 )
			pChDisplay = pDisplay[0];
			*pDisplay = "Display=%i03 C  \nHistory=%i02";
			
			// ja vain tämä taski käyttää lcd:tä
			lcd_gotoxy(0,0); // lcd-näytön alkuun
			// tulostetaan näyttö tässä silmukassa
			while(*pChDisplay != 0)
			{
				// onko muuttujan tulostuspaikka?
				if( *pChDisplay == '%')
				{
					pChDisplay++; //ohitetaan %-merkki
					// minkä tyypin dataa?
					switch(*pChDisplay)
					{
						// int-tyypin kokonaislukuja
						// muuttujan arvo poimitaan ints-taulukosta indeksin määräämästä paikasta
						case 'i': pChDisplay++;
						// indeksi annettu muodossa 09, 10,11,..
						i = (*pChDisplay - '0')*10;
						pChDisplay++;
						i +=  (*pChDisplay- '0'); // indeksi ints-taulukkoon
						taskENTER_CRITICAL(); //////////////////////////////////
						itoa(ints[i],szVariable,10);                        ////
						taskEXIT_CRITICAL();  //////////////////////////////////
						szVariable[3] = szVariable[2];
						szVariable[2] = ',';
						break;
					}
					// tulostetaan muuttuja
					pChVariable = szVariable;
					while(*pChVariable != 0)
					{
						lcd_putc(*pChVariable); // merkki näkyviin
						pChVariable++; // seuraava kirjain
					}
				}
				else if(*pChDisplay == '\n')
				{
					lcd_gotoxy(0,1);
				}
				else
				// näytön pohjateksti
				lcd_putc(*pChDisplay); // merkki näkyviin

				pChDisplay++; // seuraava kirjain
			}
			xSemaphoreGive( xDisplaySemaphore); // Vapautetaan semaphori
			break;
			
			case IDM_DISPLAY_MINMAX:
			xSemaphoreTake( xDisplaySemaphore, portMAX_DELAY ); 
			if( message.data == 0 )
			pChDisplay = pDisplay[0];
			*pDisplay = "Display=%i03 C  \nMinMax=%i02";
						
			// ja vain tämä taski käyttää lcd:tä
			lcd_gotoxy(0,0); // lcd-näytön alkuun
			// tulostetaan näyttö tässä silmukassa
			while(*pChDisplay != 0)
			{
				// onko muuttujan tulostuspaikka?
				if( *pChDisplay == '%')
				{
					pChDisplay++; //ohitetaan %-merkki
					// minkä tyypin dataa?
					switch(*pChDisplay)
					{
						// int-tyypin kokonaislukuja
						// muuttujan arvo poimitaan ints-taulukosta indeksin määräämästä paikasta
						case 'i': pChDisplay++;
						// indeksi annettu muodossa 09, 10,11,..
						i = (*pChDisplay - '0')*10;
						pChDisplay++;
						i +=  (*pChDisplay- '0'); // indeksi ints-taulukkoon
						taskENTER_CRITICAL(); //////////////////////////////////
						itoa(ints[i],szVariable,10);                        ////
						taskEXIT_CRITICAL();  //////////////////////////////////
						szVariable[3] = szVariable[2];
						szVariable[2] = ',';
						break;
					}
					// tulostetaan muuttuja
					pChVariable = szVariable;
					while(*pChVariable != 0)
					{
						lcd_putc(*pChVariable); // merkki näkyviin
						pChVariable++; // seuraava kirjain
					}
				}
				else if(*pChDisplay == '\n')
				{
					lcd_gotoxy(0,1);
				}
				else
				// näytön pohjateksti
				lcd_putc(*pChDisplay); // merkki näkyviin

				pChDisplay++; // seuraava kirjain
			}
			xSemaphoreGive( xDisplaySemaphore); // Vapautetaan semaphori
			break;
			
			case IDM_DISPLAY_TIME:
			xSemaphoreTake( xDisplaySemaphore, portMAX_DELAY ); 
			//lcd_clrscr();
			ShowTime();
			xSemaphoreGive( xDisplaySemaphore); // Vapautetaan semaphori
			break;
			
			default:
				lcd_putc('E');
				break; // virheviestit
			//         xSemaphoreGive( xPortB ); //
		}
	} //..........................
}
//--------------------- painikkeita lukeva taski -------------------------------
static void vKeyPadHandler( void *pvParameters )
{
	static unsigned char ch = 0;
	static DISPLAY_MESSAGE message;
	
	( void ) pvParameters; /* Just to stop compiler warnings. */

	// taskilla tulee aina  olla ikisilmukka
	for( ;; )//=================
	{
		do  //tämä silmukka kuormittaa!!!
		{
			xSemaphoreTake( xADC, portMAX_DELAY );
			ch =GetKey();vTaskDelay(1);
			xSemaphoreGive( xADC );

		}while (ch == NO_KEY);

		switch( ch )
		{
			case IDK_SELECT:  // näppäinten käyttö
			
			taskENTER_CRITICAL(); //////////////////////////////////
			ints[IDD_LASTKEY] = ch;                             ////
			taskEXIT_CRITICAL();  //////////////////////////////////
			// lähetään viesti lcd-näyttötaskille,jotta se päivittää näytön
			message.data      = 0; // näytön numero
			message.idMessage = IDM_UPDATE_DISPLAY;
			xQueueSend( xDisplay, (void*)&message,0);
			break;
			
			case IDK_DOWN: case IDK_UP: 
			taskENTER_CRITICAL(); //////////////////////////////////
			ints[IDD_LASTKEY] = ch;                             ////
			taskEXIT_CRITICAL();  //////////////////////////////////
			// lähetään viesti lcd-näyttötaskille,jotta se päivittää näytön
			message.data      = 0; // näytön numero
			message.idMessage = IDM_DISPLAY_MINMAX;
			xQueueSend( xDisplay, (void*)&message,0);
			break;
			
			case IDK_RIGHT: case IDK_LEFT:
			taskENTER_CRITICAL(); //////////////////////////////////
			ints[IDD_LASTKEY] = ch;                             ////
			taskEXIT_CRITICAL();  //////////////////////////////////
			// lähetään viesti lcd-näyttötaskille,jotta se päivittää näytön
			message.data      = 0; // näytön numero
			//message.idMessage = IDM_DISPLAY_HISTORY;
			message.idMessage = IDM_DISPLAY_TIME;
			xQueueSend( xDisplay, (void*)&message,0);
			break;
			
			default:
			break; // muut näppäimet
		}
	} // =========================
}




//--------------------- kelloa lukeva taski -------------------------------

////////////////////////////////////////////////////////////////
void StartTimer( int ticks)
{
	// ks. manuaalia doc2549.pdf sivulta 118 alkaen
	// 0 = stop  1 = clock  2 = clock/8 3 = clock/64  4 = clock/256 5 = clock/1024
	TCCR0B = (1<<FOC0A) | ( 1<<CS02) | (1<<CS00); // prosessorin kellotaajuus/1024   , jos 16MHz => 64 us
	TIMSK0 |= (1 << OCIE0A); // vertailuarvokeskeytys
	OCR0A = ticks; // laitetaan vertailuarvoksi annettu lukema
	TCNT0 = 0;    // laskuri alkuun
}

////////////////////////////////////////////////////////////////
SIGNAL(TIMER0_COMPA_vect) {
	static BaseType_t xTaskWoken = pdFALSE;
	static unsigned msCounter = 0;
	msCounter += 8 ; // interval 8ms
	TCNT0 = 0; // laskuri alkuun
	//timer0 interrupt handler
	if ( msCounter == 1000)
	{
		secondsFromMidNight++;
		xSemaphoreGiveFromISR( xClock, &xTaskWoken ); // ilmoitetaan kellonajan muutos
		msCounter = 0;
	}
}
////////////////////////////////////////////////////////////////

void ShowTime(void)
{
	char  szVariable[8];  // muuttujan arvo tulostetaan tähän

	lcd_gotoxy(0,0);
	// tunnit
	if( intsTime[IDD_HOUR] < 10)
	lcd_putc('0');
	itoa(intsTime[IDD_HOUR],szVariable,10);
	lcd_puts(szVariable);
	lcd_putc(':');
	// minuutit
	if( intsTime[IDD_MINUTES] < 10)
	lcd_putc('0');
	itoa(intsTime[IDD_MINUTES],szVariable,10);
	lcd_puts(szVariable);
	lcd_putc(':');
	// sekunnit
	if( intsTime[IDD_SECONDS] < 10)
	lcd_putc('0');
	itoa(intsTime[IDD_SECONDS],szVariable,10);
	lcd_puts(szVariable);
}
static void vClock( void *pvParameters )
{
	( void ) pvParameters; // Just to stop compiler warnings.

	vSemaphoreCreateBinary( xClock );// luodaan semafori

	StartTimer(125); // = 8 msekunnin välein keskeytys 8*125 = 1000ms = 1s
	
	// taskilla tulee aina  olla ikisilmukka
	for( ;; )//.........
	{
		xSemaphoreTake( xClock, portMAX_DELAY ); // odotetaan tietoa keskeytyksestä

		taskENTER_CRITICAL(); /////////////////////////////////////////
		intsTime[ IDD_HOUR ]   =  secondsFromMidNight / 3600L;         ////
		intsTime[ IDD_MINUTES ]= (secondsFromMidNight % 3600L) / 60L ; ////
		intsTime[ IDD_SECONDS ]=  secondsFromMidNight % 60L;           ////
		taskEXIT_CRITICAL(); //////////////////////////////////////////
		// näytetään aika
		//ShowTime();
	}
}

// ----------------------------------- terminaalihommat
void OwnGets(char *pText)
{
	static  unsigned char ch = 0;
	for(;;)
	{
		while( xSerialGetChar( &xSerialPort, &ch)== pdFAIL); // odotetaan kunnes saadaan merkki
		if ( ch != 13)
		{
			*pText=ch;
			pText++;
		}
		else
		{
			*pText = 0; // loppunolla
			return; // nyt jono valmis
		}
	}
}
//--------------------- näppäimistöä lukeva taski -------------------------------
static void vTerminal( void *pvParameters )
{
	static char  line[80] = {0};

	( void ) pvParameters; /* Just to stop compiler warnings. */

	xSerialxPrintf(&xSerialPort,"\r\nArduino running..\r\n");
	
	// taskilla tulee aina  olla ikisilmukka
	for( ;; )//=================
	{
	} // =========================
}
