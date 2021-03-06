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

	// initialize max and minimum numbers; according to the datasheet, they are: -55 C - 125 C
	// temperature times 10x because it's integer
	ints[IDD_MAXTEMPERATURE] = -600;
	ints[IDD_MINTEMPERATURE] = 1300;
	

	for( ;; )
	{
		if (numberOfSensors)
		{
			value  = (int)(GetTemperature(0)/1000);
			taskENTER_CRITICAL(); //////////////////////////
			ints[ IDD_TEMPERATURE ] = value ;           ////
			taskEXIT_CRITICAL(); ///////////////////////////
			
			if (value >ints[IDD_MAXTEMPERATURE])
			{
				ints[IDD_MAXTEMPERATURE] = value;
			}
			if (value <ints[IDD_MINTEMPERATURE])
			{
				ints[IDD_MINTEMPERATURE] = value;
			}
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
	static DISPLAY_MESSAGE message;

	( void ) pvParameters;	/* Just to stop compiler warnings. */


	for( ;; )
	{
		xQueueReceive(xDisplay,&message, portMAX_DELAY); // wait until a message is gotten
		
		// check if the screen is the same as the last one, to reduce the screen interrupt
		if (LastScreen != message.idMessage)
		{
			// clear screen
			lcd_clrscr();
			LastScreen = message.idMessage;
		}
		
		
		switch( message.idMessage)
		{
			// check the what %i01 means in above ints in GlobalVariables.h, %i01 = IDD_LASTKEY
			
			case IDM_UPDATE_DISPLAY:
			// prints instructions to the user
			DisplayWrite("Up  =Set Time\nDown=Set Date");
			break;
			
			case IDM_DISPLAY_MINMAX:
			// prints maximum temperature and minimum temperature
			DisplayWrite("Max: %i03 C\nMin: %i04 C");
			break;
			
			case IDM_DISPLAY_AVERAGE:
			// prints average screen
			DisplayWrite("Average Display");
			break;
			
			case IDM_DISPLAY_UPDATE_DATE:
			// prints date and allows user to modify data
			DisplayWrite("Set Date\n%d");
			break;
			
			case IDM_DISPLAY_UPDATE_TIME:
			// prints time and allows user to modify data
			DisplayWrite("Set Time\n%t");
			break;
			
			case IDM_DISPLAY_MAIN:
			// display time, date and temperature
			DisplayWrite("%n\n%i02 C");
			break;
			
			default:
			// prints an E to show error
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
	
	message.data = 0; // the display number
	
	( void ) pvParameters; /* Just to stop compiler warnings. */

	for( ;; )
	{
		// wait for half a second !! for stability
		const TickType_t xDelay = 500 / portTICK_PERIOD_MS;
		vTaskDelay(xDelay);

		do
		{
			xSemaphoreTake( xADC, portMAX_DELAY );
			ch =GetKey();vTaskDelay(1);
			xSemaphoreGive( xADC );

		}while (ch == NO_KEY);
			
		
		// reset timer for main screen
		mainScreenTimer = 0;

		switch( ch )
		{
			// send message to vLcdHandler
			// if select button is pressed			
			case IDK_SELECT:
			message.idMessage = IDM_UPDATE_DISPLAY;
			xQueueSend( xDisplay, (void*)&message,0);
			break;
			
			// if up or down is pressed
			case IDK_DOWN: case IDK_UP:
			// if the last screen was Update Display
			if (LastScreen == IDM_UPDATE_DISPLAY)
			{
				// up and down buttons have different function
				if (ch == IDK_UP)
				{
					// change time and go back to main screen
					ChangeTime();
					vTaskDelay(20); // wait for 1 second for the time to set
					message.idMessage = IDM_DISPLAY_MAIN;
				}
				else if (ch == IDK_DOWN)
				{
					// change date and go back to main screen
					ChangeDate();
					vTaskDelay(20); // wait for 1 second for the time to set
					message.idMessage = IDM_DISPLAY_MAIN;
				}
			}
			// otherwise it will show min/max screen - if it's not any of the updating screens
			else if(LastScreen != IDM_DISPLAY_UPDATE_DATE && LastScreen != IDM_DISPLAY_UPDATE_TIME)
			{
				message.idMessage = IDM_DISPLAY_MINMAX;
			}
			xQueueSend( xDisplay, (void*)&message,0);
			break;
			
			// in case right or left is pressed
			case IDK_RIGHT: case IDK_LEFT:
			// if it's not the updating screens
			if (LastScreen != IDM_DISPLAY_UPDATE_DATE && LastScreen != IDM_DISPLAY_UPDATE_TIME)
			{
				// show average
				message.idMessage = IDM_DISPLAY_AVERAGE;
				xQueueSend( xDisplay, (void*)&message,0);
			}
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
// vTerminal will print data to the Serial, programs like PuTTy can be used to communicate with the micro controller. Now it prints 10x temperature every second
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void vTerminal( void *pvParameters )
{
	char szVariable [8];
	// delay for 1000 ms
	const TickType_t xDelay = 1000 / portTICK_PERIOD_MS;

	( void ) pvParameters; /* Just to stop compiler warnings. */
	
	for( ;; )
	{
		// print temperature to Serial every second
		itoa(ints[IDD_TEMPERATURE],szVariable,10);
		xSerialxPrintf(&xSerialPort, szVariable);
		xSerialxPrintf(&xSerialPort, "\n\r");
		vTaskDelay( xDelay );
		
	} // task for end
} // task end

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// vClock is used to keep up the time while everything else is going on
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void vClock( void *pvParameters )
{
	static DISPLAY_MESSAGE message;
	( void ) pvParameters; // Just to stop compiler warnings.

	vSemaphoreCreateBinary( xClock );

	StartTimer(125); // = for every 8 ms interrupt 8*125 = 1000ms = 1s
	
	// prepare message for vLcdHandler
	message.data      = 0;
	message.idMessage = IDM_DISPLAY_MAIN;
	
	// default data for date
	intsTime[ IDD_DAY ]		= 14;
	intsTime[ IDD_MONTH ]	= 12;
	intsTime[ IDD_YEAR ]	= 2020;
	
	for( ;; )
	{
		xSemaphoreTake( xClock, portMAX_DELAY ); // wait until interrupt
		
		taskENTER_CRITICAL(); /////////////////////////////////////////////
		intsTime[ IDD_HOUR ]   =  secondsFromMidNight / 3600L;         ////
		intsTime[ IDD_MINUTES ]= (secondsFromMidNight % 3600L) / 60L ; ////
		intsTime[ IDD_SECONDS ]=  secondsFromMidNight % 60L;           ////
		taskEXIT_CRITICAL(); //////////////////////////////////////////////
		
		// when a whole day has gone by
		if(secondsFromMidNight > (24*3600L))
		{
			secondsFromMidNight = 0;
			intsTime[ IDD_DAY ]++;
		}
		// if the whole month has gone by - check how many days in current month
		if (intsTime[ IDD_DAY ] > HowManyDaysInMonth(intsTime[ IDD_MONTH ], intsTime[IDD_YEAR]))
		{
			intsTime[ IDD_DAY ] = 1;
			intsTime[ IDD_MONTH ]++;
		}
		// if the whole year has gone by
		if (intsTime[ IDD_MONTH ] > 12)
		{
			intsTime[ IDD_MONTH ] = 1;
			intsTime[ IDD_YEAR ]++;
		}
		
		
		// wait for ScreenWaitTime seconds to show the main screen
		if (mainScreenTimer >= ScreenWaitTime)
		{
			xQueueSend( xDisplay, (void*)&message,0);
		}
		
		// if the timer isn't stopped
		if (!mainScreenTimerStopped)
		{
			// add to the timer
			mainScreenTimer++;	
		}
		
		
	} // task for end
} // task end

#endif /* TASKS_H_ */