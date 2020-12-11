/****************************************************************************
 DESCRIPTION
       Basic routines for interfacing a HD44780U-based text lcd display
       with Arduino Uno
 USAGE
       See the C include lcd.h file for a description of each function

	   Arduino AtMega 2560 with LCD shield

	   28.10.2017 Initialization error fixed

*****************************************************************************/

//#define F_CPU 16000000              /**< clock frequency in Hz, used to calculate delay timer */

// atmel definitions
#include <inttypes.h>
#include <avr/pgmspace.h>
#include "device.h"

// Arduino's pin configuration. RW is always grounded.

#define LCD_DATA0_PORT   PORTG     /**< port for 4bit data bit 0 */
#define LCD_DATA1_PORT   PORTE     /**< port for 4bit data bit 1 */
#define LCD_DATA2_PORT   PORTH     /**< port for 4bit data bit 2 */
#define LCD_DATA3_PORT   PORTH     /**< port for 4bit data bit 3 */
#define LCD_DATA0_PIN    5         /**< pin for 4bit data bit 0  */
#define LCD_DATA1_PIN    3         /**< pin for 4bit data bit 1  */
#define LCD_DATA2_PIN    3         /**< pin for 4bit data bit 2  */
#define LCD_DATA3_PIN    4         /**< pin for 4bit data bit 3  */
#define LCD_RS_PORT      PORTH     /**< port for RS line         */
#define LCD_RS_PIN       5         /**< pin  for RS line         */
//#define LCD_RW_PORT      PORTB   /**< port for RW line         */
//#define LCD_RW_PIN       5       /**< pin  for RW line         */
#define LCD_E_PORT       PORTH     /**< port for Enable line     */
#define LCD_E_PIN        6         /**< pin  for Enable line     */

/*
** constants/macros
*/
#define PIN(x) (*(&x - 2))  /* address of data direction register of port x */
#define DDR(x) (*(&x - 1))  /* address of input register of port x          */


#define lcd_e_high()    LCD_E_PORT  |=  _BV(LCD_E_PIN)
#define lcd_e_low()     LCD_E_PORT  &= ~_BV(LCD_E_PIN)
//#define lcd_rw_high()   LCD_RW_PORT |=  _BV(LCD_RW_PIN)
//#define lcd_rw_low()    LCD_RW_PORT &= ~_BV(LCD_RW_PIN)
#define lcd_rs_high()   LCD_RS_PORT |=  _BV(LCD_RS_PIN)
#define lcd_rs_low()    LCD_RS_PORT &= ~_BV(LCD_RS_PIN)


/*************************************************************************
Low-level function to write byte to LCD controller
Input:    data   byte to write to LCD
          rs     1: write data
                 0: write instruction
          oln    1: only low nibble
Returns:  none
*************************************************************************/
static void lcd_write(uint8_t data,uint8_t rs,uint8_t oln)
{
    if (rs)
	{   /* write data        (RS=1, RW=0) */
       lcd_rs_high();
    }
	else
	{    /* write instruction (RS=0, RW=0) */
       lcd_rs_low();
    }

    if (oln == 0)
	{
      // output high nibble first
	  if(data & 0x80) LCD_DATA3_PORT |= _BV(LCD_DATA3_PIN);
	  else LCD_DATA3_PORT &= ~_BV(LCD_DATA3_PIN);
	  if(data & 0x40) LCD_DATA2_PORT |= _BV(LCD_DATA2_PIN);
	  else LCD_DATA2_PORT &= ~_BV(LCD_DATA2_PIN);
	  if(data & 0x20) LCD_DATA1_PORT |= _BV(LCD_DATA1_PIN);
	  else LCD_DATA1_PORT &= ~_BV(LCD_DATA1_PIN);
	  if(data & 0x10) LCD_DATA0_PORT |= _BV(LCD_DATA0_PIN);
	  else LCD_DATA0_PORT &= ~_BV(LCD_DATA0_PIN);

	  // pulse enable
      lcd_e_low() ;
	  _delay_us(1);     //    ___
      lcd_e_high() ;   // ___   ___
	  _delay_us(1);
      lcd_e_low() ;
 	  _delay_ms(1);
    }

    // output low nibble
    if(data & 0x08) LCD_DATA3_PORT |= _BV(LCD_DATA3_PIN);
	else LCD_DATA3_PORT &= ~_BV(LCD_DATA3_PIN);

   	if(data & 0x04) LCD_DATA2_PORT |= _BV(LCD_DATA2_PIN);
	else LCD_DATA2_PORT &= ~_BV(LCD_DATA2_PIN);
   	if(data & 0x02) LCD_DATA1_PORT |= _BV(LCD_DATA1_PIN);
    else LCD_DATA1_PORT &= ~_BV(LCD_DATA1_PIN);
   	if(data & 0x01) LCD_DATA0_PORT |= _BV(LCD_DATA0_PIN);
    else LCD_DATA0_PORT &= ~_BV(LCD_DATA0_PIN);

    lcd_e_low() ;
	_delay_us(1);   //   ___
    lcd_e_high() ; //___    ___
    _delay_us(1);
    lcd_e_low() ;
 	_delay_ms(1);

    // all data pins high (inactive)
    LCD_DATA0_PORT |= _BV(LCD_DATA0_PIN);
    LCD_DATA1_PORT |= _BV(LCD_DATA1_PIN);
    LCD_DATA2_PORT |= _BV(LCD_DATA2_PIN);
    LCD_DATA3_PORT |= _BV(LCD_DATA3_PIN);
}

//--- PUBLIC FUNCTIONS --------------------------------------------------


// ----------------------------------------------------------------------
// Send LCD controller instruction command
// Input:   instruction to send to LCD controller, see HD44780 data sheet
// Returns: none
// ----------------------------------------------------------------------
void lcd_command(uint8_t cmd)
{
    lcd_write(cmd,0,0);
}


// ----------------------------------------------------------------------
// Set cursor to specified position
// Input:   x  horizontal position  (0: left most position)
//          y  vertical position    (0: first line)
// Returns:  none
// ----------------------------------------------------------------------
void lcd_gotoxy(uint8_t x, uint8_t y)
{
  uint8_t i;
   lcd_command(1<<LCD_HOME); // goto home
   if (y==1)
     x += 40;
    for(i=0; i < x; i++)
	  lcd_command(LCD_MOVE_CURSOR_RIGHT);
}

// ----------------------------------------------------------------------
// Clear display and set cursor to home position
// ----------------------------------------------------------------------
void lcd_clrscr(void)
{
    lcd_command(1<<LCD_CLR);
    _delay_ms(3);     //  wait 2.16 ms or more
}
// ----------------------------------------------------------------------
// Set cursor to home position
// ----------------------------------------------------------------------
void lcd_home(void)
{
    lcd_command(1<<LCD_HOME);
    _delay_ms(3);     //  wait 2.16 ms or more  

}

// ----------------------------------------------------------------------
// Display character at current cursor position
// Input:    character to be displayed
// Returns:  none
// ----------------------------------------------------------------------
void lcd_putc(char c)
{
    
	lcd_write(c, 1,0);
}

// ----------------------------------------------------------------------
// Display string without auto linefeed
// Input:    string to be displayed
// Returns:  none
// ----------------------------------------------------------------------
void lcd_puts(const char *s)
/* print string on lcd (no auto linefeed) */
{
    register char c;

    while ( (c = *s++) ) {
        lcd_putc(c);
    }

}


/*************************************************************************
Display string from program memory without auto linefeed
Input:     string from program memory be be displayed
Returns:   none
*************************************************************************/
void lcd_puts_p(const char *progmem_s)
/* print string from program memory on lcd (no auto linefeed) */
{
    register char c;

    while ( (c = pgm_read_byte(progmem_s++)) )
        lcd_putc(c);

}

/*************************************************************************
Initialize display and select type of cursor
Input:    dispAttr LCD_DISP_OFF            display off
                   LCD_DISP_ON             display on, cursor off
                   LCD_DISP_ON_CURSOR      display on, cursor on
                   LCD_DISP_CURSOR_BLINK   display on, cursor on flashing
Returns:  none
*************************************************************************/
void lcd_init(uint8_t dispAttr)
{
   //
   //  Initialize LCD to 4 bit I/O mode
   //

    // configure all port bits as output
   DDR(LCD_RS_PORT)    |= _BV(LCD_RS_PIN);
   //DDR(LCD_RW_PORT)    |= _BV(LCD_RW_PIN);
   DDR(LCD_E_PORT)     |= _BV(LCD_E_PIN);
   DDR(LCD_DATA0_PORT) |= _BV(LCD_DATA0_PIN);
   DDR(LCD_DATA1_PORT) |= _BV(LCD_DATA1_PIN);
   DDR(LCD_DATA2_PORT) |= _BV(LCD_DATA2_PIN);
   DDR(LCD_DATA3_PORT) |= _BV(LCD_DATA3_PIN);
	
   _delay_ms(60);     //  wait 50ms or more after power-on

   // reset the lcd circuit
 //  lcd_write(0x33, 0,0);
 //  _delay_ms(5);      // wait 5 ms
    lcd_rs_low();
    lcd_e_low();


   // set the device to the 4-bit mode
   lcd_write(0x3, 0,1);
   _delay_ms(5);      // wait 5 ms
   // set the device to the 4-bit mode
   lcd_write(0x3, 0,1);
   _delay_ms(1);      // wait 1 ms
   // set the device to the 4-bit mode
   lcd_write(0x3, 0,1);
   _delay_ms(1);      // wait 5 ms
   // finally, set to 4-bit interface
   lcd_write(0x02,0,1);

   // set 2 lines and 5x7 dots /character mode
   // NFXX
   //   N = 1 2 lines  F = 1 5x10   F = 0 5x7
   lcd_write(0x28,0,0);

   // from now the LCD only accepts 4 bit I/O, we can use lcd_command()
    lcd_clrscr();                     // display clear
    lcd_command(LCD_MODE_DEFAULT);    // set entry mode
	lcd_command(dispAttr);

}






