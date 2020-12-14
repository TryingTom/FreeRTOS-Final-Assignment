#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

////////////////////////////////////////////////////////////////
//
// StartTimer starts a time with given reference ticks
//
////////////////////////////////////////////////////////////////
void StartTimer( int ticks)
{
	// 0 = stop  1 = clock  2 = clock/8 3 = clock/64  4 = clock/256 5 = clock/1024
	TCCR0B = (1<<FOC0A) | ( 1<<CS02) | (1<<CS00); // Processors clock/1024   , if 16MHz => 64 us
	TIMSK0 |= (1 << OCIE0A); // reference interrupt
	OCR0A = ticks; // use the given number as a reference
	TCNT0 = 0;    // to the start of the counter
}

////////////////////////////////////////////////////////////////
//
// Timer
//
////////////////////////////////////////////////////////////////
SIGNAL(TIMER0_COMPA_vect) {
	static BaseType_t xTaskWoken = pdFALSE;
	static unsigned msCounter = 0;
	msCounter += 8 ; // interval 8ms
	TCNT0 = 0; // to the start of the counter
	//timer0 interrupt handler
	if ( msCounter == 1000)
	{
		secondsFromMidNight++;
		xSemaphoreGiveFromISR( xClock, &xTaskWoken ); // notify the change in time
		msCounter = 0;
	}
}

////////////////////////////////////////////////////////////////
//
// ShowTime will print the time on the screen
//
////////////////////////////////////////////////////////////////
void ShowTime(void)
{
	char  szVariable[8];  // variable value will be printed here

	// hours
	if( intsTime[IDD_HOUR] < 10)
	lcd_putc('0');
	itoa(intsTime[IDD_HOUR],szVariable,10);
	lcd_puts(szVariable);
	lcd_putc(':');
	// minutes
	if( intsTime[IDD_MINUTES] < 10)
	lcd_putc('0');
	itoa(intsTime[IDD_MINUTES],szVariable,10);
	lcd_puts(szVariable);
	lcd_putc(':');
	// seconds
	if( intsTime[IDD_SECONDS] < 10)
	lcd_putc('0');
	itoa(intsTime[IDD_SECONDS],szVariable,10);
	lcd_puts(szVariable);
}

////////////////////////////////////////////////////////////////
//
// ShowTimeHoursAndMinutesAndDate will print the hour and minute on the screen, and the date after that
//
////////////////////////////////////////////////////////////////

void ShowTimeHoursAndMinutesAndDate(void)
{
	char  szVariable[8];  // variable value will be printed here

	//lcd_gotoxy(0,0);
	// hours
	if( intsTime[IDD_HOUR] < 10)
	lcd_putc('0');
	itoa(intsTime[IDD_HOUR],szVariable,10);
	lcd_puts(szVariable);
	lcd_putc(':');
	// minutes
	if( intsTime[IDD_MINUTES] < 10)
	lcd_putc('0');
	itoa(intsTime[IDD_MINUTES],szVariable,10);
	lcd_puts(szVariable);
	lcd_putc(' ');
	// day
	if(intsTime[IDD_DAY]<10)
	lcd_putc('0');
	itoa(intsTime[IDD_DAY],szVariable,10);
	lcd_puts(szVariable);
	lcd_putc('.');
	// month
	if(intsTime[IDD_MONTH]<10)
	lcd_putc('0');
	itoa(intsTime[IDD_MONTH],szVariable,10);
	lcd_puts(szVariable);
	lcd_putc('.');
	// year
	itoa(intsTime[IDD_YEAR],szVariable,10);
	lcd_puts(szVariable);
	
}

////////////////////////////////////////////////////////////////
//
// ShowDate will print the date
//
////////////////////////////////////////////////////////////////

void ShowDate(void)
{
	char  szVariable[8];  // variable value will be printed here

	// day
	if(intsTime[IDD_DAY]<10)
	lcd_putc('0');
	itoa(intsTime[IDD_DAY],szVariable,10);
	lcd_puts(szVariable);
	lcd_putc('.');
	// month
	if(intsTime[IDD_MONTH]<10)
	lcd_putc('0');
	itoa(intsTime[IDD_MONTH],szVariable,10);
	lcd_puts(szVariable);
	lcd_putc('.');
	// year
	itoa(intsTime[IDD_YEAR],szVariable,10);
	lcd_puts(szVariable);
	
}

void DisplayWrite(char *text)
{
	char *pDisplay[] = {text};
	volatile  char *pChDisplay =0,  // pointer for *pDisplay, shows the current character
	*pChVariable=0; // pointer which is connected to variable value printing
	int             i;
	char            szVariable[8]; // variable value is printed here
	bool skipVariablePrint = false;
	
	xSemaphoreTake( xDisplaySemaphore, portMAX_DELAY ); // Take Semaphore for uninterrupted printing
	pChDisplay = pDisplay[0];
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
				// todo: fix the comma problem on the temperature...
				skipVariablePrint = false;
				break;
				
				// case: now
				case 'n':
				taskENTER_CRITICAL(); //////////////////////////////////
				ShowTimeHoursAndMinutesAndDate();				////////
				taskEXIT_CRITICAL();  //////////////////////////////////
				skipVariablePrint = true;
				break;
				
				// case: date
				case 'd':
				taskENTER_CRITICAL(); //////////////////////////////////
				ShowDate();										////////
				taskEXIT_CRITICAL();  //////////////////////////////////
				skipVariablePrint = true;
				break;
				
				// case: time
				case 't':
				taskENTER_CRITICAL(); //////////////////////////////////
				ShowTime();										////////
				taskEXIT_CRITICAL();  //////////////////////////////////
				skipVariablePrint = true;
				break;
			}
			if(!skipVariablePrint)
			{
				// the variable is printed
				pChVariable = szVariable;
				while(*pChVariable != 0)
				{
					pChVariable++;
					if(*pChVariable == 0)
					{
						lcd_putc('.');
					}
					pChVariable--;
					lcd_putc(*pChVariable); // print the character
					pChVariable++;			// next character
				}
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
}

#endif /* FUNCTIONS_H_ */