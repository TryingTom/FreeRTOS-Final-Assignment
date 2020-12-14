//--------------------------------------------------------------
//	Used as a reference:
//  "Realiaikajärjestelmät      syksy 2017   ktMakva 3.10.2017  FreeRTOS on Arduino Atmega 2560"
//
//    Devices used: Arduino Mega 2560 + DFRobot LCD Keypad Shield V1.1 + DS1820
//
//	The program was made by TryingTom (using libraries from other people) 
//	https://github.com/TryingTom/FreeRTOS-Final-Assignment

/*----------------------------------------------------------------------------------------------------------------------------------
The program displays time, date and temperature on LCD screen.
It uses FreeRTOS to make everything work real-time and work alongside each other.

The used LCD screen has 5 buttons, four directional and 1 select. 

The main screen shows the time, date and temperature.
If up or down is pressed, the user can move to next screen. The main screen will show again if nothing is pressed for 3 seconds.
There are following screens implemented:
 - screen will show minimum and maximum values and their time
 - screen will show average by the hour, showing also the max and minimum for that hour
 - screen will allow you to set time or set date
	- set time screen
	- set date screen
	
There will be also USB connection to another program on the computer, which shows user interface for the different temperatures according to the time.
	
	
/////////////////////////////////
Todo:
Fixing screen texts
Implement change time
Implement display date
Implement change date
Implement calculate average

UI for the the data
/////////////////////////////////
----------------------------------------------------------------------------------------------------------------------------------*/



#define F_CPU 16000000L

// Scheduler include files. --------------------------------------------
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
#include "semphr.h"
#include "serial.h"
#include "queue.h"
#include "event_groups.h"
// avr needed for timer
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// c language functions
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
// local devices
#include "../ds1820/sensor.h"   // functions for the temperature sensor
#include "..\devices\device.h"  // LCD-panel

#include "GlobalVariables.h"
#include "Tasks.h"
#include "Functions.h"

//--------------------------------------------------------
// main creates tasks and starts the system
int main( void )
{
	xDisplay = xQueueCreate( 2, sizeof(DISPLAY_MESSAGE));// queue initialized for display task

	// semaphores initialized
	vSemaphoreCreateBinary( xADC );
	vSemaphoreCreateBinary( xDisplaySemaphore);

	// initialize serial port
    xSerialPort = xSerialPortInitMinimal(0, mainCOM_BAUD_RATE, comBUFFER_LEN ,10);
	// initialize LCD
	lcd_init(LCD_DISP_ON);  

	// create tasks
	xTaskCreate( vLcdHandler, 0, configMINIMAL_STACK_SIZE, 0,  (tskIDLE_PRIORITY + 2), NULL );
	xTaskCreate( vDoMeasurements, 0, configMINIMAL_STACK_SIZE, 0,  (tskIDLE_PRIORITY + 2), NULL );
	xTaskCreate( vKeyPadHandler, 0, configMINIMAL_STACK_SIZE, 0,  (tskIDLE_PRIORITY + 2), NULL );
	
	xTaskCreate( vClock, 0, configMINIMAL_STACK_SIZE, 0,  (tskIDLE_PRIORITY + 2), NULL );
	xTaskCreate( vTerminal, 0, configMINIMAL_STACK_SIZE, 0,  (tskIDLE_PRIORITY + 2), NULL );
	
	// turn on timer
	vTaskStartScheduler(); 

	return 0;
}