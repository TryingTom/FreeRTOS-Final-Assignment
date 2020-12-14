//--------------------------------------------------------------
//	Used as a reference:
//  "Realiaikajärjestelmät      syksy 2017   ktMakva 3.10.2017  FreeRTOS on Arduino Atmega 2560"
//
//    Devices used: Arduino Mega 2560 + DFRobot LCD Keypad Shield V1.1 + DS1820
//
//	The program was made by TryingTom (using libraries from other people) 
//	https://github.com/TryingTom/FreeRTOS-Final-Assignment

/*----------------------------------------------------------------------------------------------------------------------------------

You can find how to use the program from the README inside the github page.
	
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