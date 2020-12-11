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

	lcd_gotoxy(0,0);
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

#endif /* FUNCTIONS_H_ */