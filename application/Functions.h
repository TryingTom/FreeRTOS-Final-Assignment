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

////////////////////////////////////////////////////////////////
//
// DisplayWrite will print the given text onto the display
//
////////////////////////////////////////////////////////////////

void DisplayWrite(char *text)
{
	char *pDisplay[] = {text};
	volatile  char *pChDisplay =0,  // pointer for *pDisplay, shows the current character
	*pChVariable=0;					// pointer which is connected to variable value printing
	int             i;
	char            szVariable[8];	// variable value is printed here
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

////////////////////////////////////////////////////////////////
//
// ReadKeyPadWithLCD allows the user to go between 0 and maximum value given, then it prints the value on LCD screen.
//
////////////////////////////////////////////////////////////////

int  ReadKeyPadWithLCD(char *szPrompt, int nMax)
{
	char text[10]={0};
	int nValue=0;
	int key;
	char *p = szPrompt;

	// show prompt
	lcd_clrscr();
	while(*p )
	{
		if(*p == '\n')
		lcd_gotoxy(0,1);
		else
		lcd_putc(*p);
		p++;
	}
	do
	{
		while( (key = GetKey()) == NO_KEY); // wait for press
		if (key == IDK_UP)
		{
			if(nValue < nMax)
			nValue++;
			// if the value raises to the maximum value, it goes back to zero, allowing looping the values
			else if (nValue == nMax)
			nValue = 0;
			
		}
		else if(key == IDK_DOWN)
		{
			if(nValue)
			nValue--;
			// if the value lowers to the minimum value, it goes back to max, allowing looping the values
			else if(!nValue)
			nValue = nMax;
		}
		else if(key == IDK_SELECT)
		{
			while( (key = GetKey()) != NO_KEY); // wait for the button to be ready
			return nValue;
		}
		// print the number
		itoa(nValue,text,10);
		// add 0 if less than 10
		if (nValue<10)
		{
			lcd_gotoxy(10,1);
			lcd_putc('0');
			lcd_gotoxy(11,1);	
		}
		else
		{
			lcd_gotoxy(10,1);
		}
		
		lcd_puts(text);
		
		while( (key = GetKey()) != NO_KEY); // wait for button to be ready

	}
	while(1);

}

////////////////////////////////////////////////////////////////
//
// ChangeTime uses the ReadKeyPadWithLCD() function to write text and get the returned time, putting it to the database.
//
////////////////////////////////////////////////////////////////

void ChangeTime (void)
{
	unsigned int hour, minutes,seconds;
	
	// stop the timer so the user has time to set the time
	mainScreenTimerStopped = true;
	xSemaphoreTake( xADC, portMAX_DELAY ); // take semaphore
	hour = ReadKeyPadWithLCD("TIME SET\nHours? ",23);
	vTaskDelay(20);
	minutes = ReadKeyPadWithLCD("TIME SET\nMinutes? ",59);
	vTaskDelay(20);
	seconds = ReadKeyPadWithLCD("TIME SET\nSeconds? ",59);
	vTaskDelay(20);
	
	// set time
	secondsFromMidNight = hour*3600L+minutes*60L+seconds;
	intsTime[ IDD_HOUR ]	= hour;
	intsTime[ IDD_MINUTES ]	= minutes;
	intsTime[ IDD_SECONDS ]	= seconds;
	
	vTaskDelay(20); // wait 1 second
	// allow the timer to run again
	mainScreenTimerStopped = false;
	xSemaphoreGive( xADC ); // give semaphore
}

////////////////////////////////////////////////////////////////
//
// ChangeDate uses the ReadKeyPadWithLCD() function to write text and get the returned date, putting it to the database.
//
////////////////////////////////////////////////////////////////

void ChangeDate(void)
{
	unsigned int year, month, day;
	
	// stop the timer so the user has time to set the time
	mainScreenTimerStopped = true;
	xSemaphoreTake( xADC, portMAX_DELAY ); // take semaphore
	year = ReadKeyPadWithLCD("DATE SET\nYear?   2000", 99);
	vTaskDelay(20);
	month = ReadKeyPadWithLCD("DATE SET\nMonth? ", 12);
	if (month == 0)
	{
		month = 1;
	}
	vTaskDelay(20);
	day = ReadKeyPadWithLCD("DATE SET\nDay? ", HowManyDaysInMonth(month, year));
	if (day == 0)
	{
		day = 1;
	}
	vTaskDelay(20);
	
	// set date
	intsTime[ IDD_DAY ]		= day;
	intsTime[ IDD_MONTH ]	= month;
	intsTime[ IDD_YEAR ]	= 2000 + year;
	
	vTaskDelay(20); // wait 1 second
	// allow the timer to run again
	mainScreenTimerStopped = false;
	xSemaphoreGive( xADC ); //give semaphore
}

////////////////////////////////////////////////////////////////
//
// HowManyDaysInMonth returns how many days are in a given month - it takes into consideration also the leap years.
//
////////////////////////////////////////////////////////////////

int HowManyDaysInMonth(int month, int year)
{
	int days = 0;
	int monthNumber = month - 1; // since enum starts from index 0
	
	if (monthNumber == eApril || monthNumber == eJune || monthNumber == eSeptember || monthNumber == eNovember)
	{
		days = 30;
	}
	// if February 
	else if (monthNumber == eFebruary)
	{
		// if the year is a leap year
		if (!(year % 4))
		{
			days = 29;
		}
		else
		{
			days = 28;
		}
	}
	// other months
	else{
		days = 31;
	}
	
	return days;
}

#endif /* FUNCTIONS_H_ */