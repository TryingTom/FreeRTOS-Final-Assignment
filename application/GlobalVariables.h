#ifndef GLOBALVARIABLES_H_
#define GLOBALVARIABLES_H_

// (IDM = identification of message)
#define IDM_UPDATE_DISPLAY		1
#define IDM_DISPLAY_MINMAX		2
#define IDM_DISPLAY_AVERAGE		3
#define IDM_DISPLAY_HISTORY		4
#define IDM_DISPLAY_TIME		5
#define IDM_DISPLAY_UPDATE_TIME	6
#define IDM_DISPLAY_UPDATE_DATE 7
// structure of the message
typedef struct
{
	char idMessage; // viestin tunniste
	char data;      // dataa
}DISPLAY_MESSAGE;


// -- introducing the tasks ---------------
static void vLcdHandler( void *pvParameters );
static void vKeyPadHandler( void *pvParameters );
static void vDoMeasurements( void *pvParameters );
static void vClock( void *pvParameters );
static void vTerminal( void *pvParameters );

/* Baud rate used by the serial port tasks. */
#define mainCOM_BAUD_RATE			(9600)
#define comBUFFER_LEN				(50)

//-- tasks global variable -------------------------------
unsigned  secondsFromMidNight = 0;
unsigned  mainScreenTimer = 5;	// initialize with >3 so the main screen shows immediately
unsigned LastScreen = 0;			// last screen so the screen isn't cleared as much
#define ScreenWaitTime 3		// how many seconds until the program goes back to the main screen
#define INTS_MAX_TIME 6
#define INTS_MAX 5

// (IDD = identification of data)
//#define IDD_LASTKEY 1
#define IDD_TEMPERATURE 2
#define IDD_MAXTEMPERATURE 3
#define IDD_MINTEMPERATURE 4
int ints[INTS_MAX] = {0}; 

#define IDD_HOUR    0
#define IDD_MINUTES 1
#define IDD_SECONDS 2
#define IDD_DAY     3
#define IDD_MONTH   4
#define IDD_YEAR    5
int intsTime[INTS_MAX_TIME] = {0};


//--------------------------------------------------------
// Queue handle
static QueueHandle_t xDisplay; // Display queue handler
// semaphores
static SemaphoreHandle_t xADC;				// Semaphore for ADC
static SemaphoreHandle_t xDisplaySemaphore; // Semaphore for Display
static SemaphoreHandle_t xClock;			// Semaphore for clock

xComPortHandle xSerialPort;

// introducing normal functions
void StartTimer( int ticks);
void ShowTime(void);
void ShowTimeHoursAndMinutesAndDate(void);
void OwnGets(char *pText);
void DisplayWrite(char *text);

#endif /* GLOBALVARIABLES_H_ */