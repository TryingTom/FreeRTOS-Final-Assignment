#ifndef TASKS_H_
#define TASKS_H_

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// vDoMeasurements will measure the temperature; in this code, the DS1820 uses external power and the data pin is connected to A7 on Arduino Mega
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void vDoMeasurements( void *pvParameters )
{
	( void ) pvParameters; // Just to stop compiler warnings.
	int value ;

	static unsigned char  numberOfSensors;
	
	numberOfSensors = GetSensorCount(); // gets the sensor count that are in the same connection


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
	} //task for end
}// task end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// vLcdHandler handles the LCD, showing the given information on the screen
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void vLcdHandler( void *pvParameters )
{

	char *pDisplay[] = {"VALUE=%i03   \nBUTTON=%i02"};
	volatile  char *pChDisplay =0,  // pointer for *pDisplay, shows the current character
	*pChVariable=0; // pointer which is connected to variable value printing
	int             i;
	char            szVariable[8]; // variable value is printed here
	static DISPLAY_MESSAGE message;

	( void ) pvParameters;	/* Just to stop compiler warnings. */


	for( ;; )
	{
		xQueueReceive(xDisplay,&message, portMAX_DELAY); // wait until a message is gotten
		
		switch( message.idMessage)
		{
			case IDM_UPDATE_DISPLAY:
			xSemaphoreTake( xDisplaySemaphore, portMAX_DELAY ); // Take Semaphore for uninterrupted printing
			if( message.data == 0 )
			pChDisplay = pDisplay[0];
			
			// initialized with a dummy text
			// todo: make this according to the assigment
			*pDisplay = "Update=%i03 C  \nDisplay=%i02";
			
			lcd_gotoxy(0,0); // to the beginning of LCD
			//// the array is printed in this loop
			while(*pChDisplay != 0)
			{
				// if it's variable printing time
				if( *pChDisplay == '%')
				{
					pChDisplay++; // move past %
					// check the type
					switch(*pChDisplay)
					{
						// int-type whole numbers
						// variable value is picked from ints-array depending on the index
						case 'i': pChDisplay++;
						// index is given in a form of 09, 10,11,..
						i = (*pChDisplay - '0')*10;
						pChDisplay++;
						i +=  (*pChDisplay- '0'); // index to ints-array
						taskENTER_CRITICAL(); //////////////////////////////////
						itoa(ints[i],szVariable,10);                        //// // itoa transfers int to string inside the szVariable array
						taskEXIT_CRITICAL();  //////////////////////////////////
						szVariable[3] = szVariable[2];
						szVariable[2] = ',';
						break;
					}
					// the variable is printed
					pChVariable = szVariable;
					while(*pChVariable != 0)
					{
						lcd_putc(*pChVariable); // print the character
						pChVariable++;			// next character
					}
				}
				else if(*pChDisplay == '\n')
				{
					lcd_gotoxy(0,1);
				}
				else
				// screen *pChDisplay text
				lcd_putc(*pChDisplay);	// print the character
				pChDisplay++;			// next character
			}
			xSemaphoreGive( xDisplaySemaphore); // give the semaphore
			break;
			
			// others are just copies, for now. 
			case IDM_DISPLAY_HISTORY:
			xSemaphoreTake( xDisplaySemaphore, portMAX_DELAY );
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
			//lcd_clrscr(); // clear screen
			ShowTime();
			xSemaphoreGive( xDisplaySemaphore); 
			break;
			
			default:
			lcd_putc('E');
			break; // error messages
		}
	} // task for end
} // task end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// vKeyPadHandler handles the button presses
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void vKeyPadHandler( void *pvParameters )
{
	static unsigned char ch = 0;
	static DISPLAY_MESSAGE message;
	
	( void ) pvParameters; /* Just to stop compiler warnings. */

	for( ;; )
	{
		do 
		{
			xSemaphoreTake( xADC, portMAX_DELAY );
			ch =GetKey();vTaskDelay(1);
			xSemaphoreGive( xADC );

		}while (ch == NO_KEY);

		switch( ch )
		{
			case IDK_SELECT:			
			taskENTER_CRITICAL(); //////////////////////////////////
			ints[IDD_LASTKEY] = ch;                             ////
			taskEXIT_CRITICAL();  //////////////////////////////////
			// send message to vLcdHandler
			message.data      = 0; // the display number
			message.idMessage = IDM_UPDATE_DISPLAY;
			xQueueSend( xDisplay, (void*)&message,0);
			break;
			
			case IDK_DOWN: case IDK_UP:
			taskENTER_CRITICAL(); //////////////////////////////////
			ints[IDD_LASTKEY] = ch;                             ////
			taskEXIT_CRITICAL();  //////////////////////////////////
			message.data      = 0;
			message.idMessage = IDM_DISPLAY_MINMAX;
			xQueueSend( xDisplay, (void*)&message,0);
			break;
			
			case IDK_RIGHT: case IDK_LEFT:
			taskENTER_CRITICAL(); //////////////////////////////////
			ints[IDD_LASTKEY] = ch;                             ////
			taskEXIT_CRITICAL();  //////////////////////////////////
			message.data      = 0;
			//message.idMessage = IDM_DISPLAY_HISTORY;
			message.idMessage = IDM_DISPLAY_TIME;
			xQueueSend( xDisplay, (void*)&message,0);
			break;
			
			default:
			break; // Other buttons
		}
	} // task for end
} // task end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// OwnGets is used to get character arrays
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void OwnGets(char *pText)
{
	static  unsigned char ch = 0;
	for(;;)
	{
		while( xSerialGetChar( &xSerialPort, &ch)== pdFAIL); // wait until a character is given
		if ( ch != 13) // if not carriage return (enter)
		{
			*pText=ch;
			pText++;
		}
		else
		{
			*pText = 0; // finish the array with 0
			return; // return, the array is saved due the pointer
		}
	} // task for end
}// task end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// vTerminal will print data to the Serial, programs like PuTTy can be used to communicate with the micro controller
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void vTerminal( void *pvParameters )
{
	static char  line[80] = {0};

	( void ) pvParameters; /* Just to stop compiler warnings. */

	xSerialxPrintf(&xSerialPort,"\r\nArduino running..\r\n");
	
	for( ;; )
	{
		// todo
	} // task for end
} // task end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// vClock is used to keep up the time while everything else is going on
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void vClock( void *pvParameters )
{
	( void ) pvParameters; // Just to stop compiler warnings.

	vSemaphoreCreateBinary( xClock );

	StartTimer(125); // = for every 8 ms interrupt 8*125 = 1000ms = 1s
	
	for( ;; )
	{
		xSemaphoreTake( xClock, portMAX_DELAY ); // odotetaan tietoa keskeytyksestä

		taskENTER_CRITICAL(); /////////////////////////////////////////
		intsTime[ IDD_HOUR ]   =  secondsFromMidNight / 3600L;         ////
		intsTime[ IDD_MINUTES ]= (secondsFromMidNight % 3600L) / 60L ; ////
		intsTime[ IDD_SECONDS ]=  secondsFromMidNight % 60L;           ////
		taskEXIT_CRITICAL(); //////////////////////////////////////////
	} // task for end
} // task end

#endif /* TASKS_H_ */