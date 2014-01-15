/*********************************************************************\
* Copyleft (>) 2014 Roman 'Fallout' Ilichev <fxdteam@gmail.com>      *
 *                                                                   *
 * This file is part of OpenXHC project                              *
 *                             WTFPL LICENSE v2                      *
\*********************************************************************/

#include <stdlib.h>
#include "stm32f10x.h"
#include "spi_master.h"
#include "io_macro.h"

#include "nokia_lcd.h"
/* nokia 5100 driver */

/* 
  HW connection: 
  XX  - LED ( NOT USED )
  PA5 - SCLK
  PA7 - DN ( MOSI )
  PA3 - D/C
  PA2 - RST
  PA4 - SCE ( CS )
  GND
  VDD
*/

extern const uint8_t font5x8[][5];

#define LCD_RESET       A, 2, SPEED_50MHz
#define LCD_CS          A, 4, SPEED_50MHz
#define LCD_DC          A, 3, SPEED_50MHz

#define SELECT()        PIN_LOW( LCD_CS )
#define UNSELECT()      PIN_HI( LCD_CS )

#define LCD_RES_ON()    PIN_LOW( LCD_RESET )
#define LCD_RES_OFF()   PIN_HI( LCD_RESET )


void nokia_hw_init( void )
{
  spi_init( 1 );
  
  PORT_ENABLE_CLOCK( LCD_RESET );
  PORT_ENABLE_CLOCK( LCD_CS );
  PORT_ENABLE_CLOCK( LCD_DC );
  
  PIN_LOW( LCD_RESET );
  PIN_OUT_PP( LCD_RESET );
  
  PIN_HI( LCD_CS );
  PIN_OUT_PP( LCD_CS );
  
  PIN_OUT_PP( LCD_DC );
}

void lcd_write_byte( uint8_t data, char cmd )
{
  if( cmd )
  {
    PIN_HI( LCD_DC );
  }
  else
  {
    PIN_LOW( LCD_DC );
  }
  
  spi1_send_byte( data );
}

void nokia_lcd_init( void )
{
  int i;
  
  LCD_RES_ON();
  i = 32000;
  while( i-- )
  {
    asm("nop");
  }
  LCD_RES_OFF();
  SELECT();
  
  /* init display */
  lcd_write_byte(0x21,0);
  lcd_write_byte(0xB0,0);
  lcd_write_byte(0x04,0);
  lcd_write_byte(0x14,0);
    
  /* we must send 0x20 before modifying the display control mode */
  lcd_write_byte( 0x20, 0);
  /* set display control, normal mode. 0x0D for inverse */
  lcd_write_byte( 0x0C, 0);
  
  UNSELECT();  
}

void lcd_set_xy( char x, char  y )
{
  lcd_write_byte(0x40 | y, 0);// column
  lcd_write_byte(0x80 | x, 0);// row
} 

void lcd_clear(void)
{
  uint16_t n;

  SELECT();
  lcd_set_xy(0,0);
  n = (6u*84u);
  while( n-- )
  { 
    lcd_write_byte(0x00,1);	 						
  }
  UNSELECT();
}

void lcd_write_char( char c)
{
  char line, n;
  c-= 32;
  if( c == 0 )
    n = 3;
  else
    n = 5;
  for (line=0; line<n; line++)
  lcd_write_byte(font5x8[c][line], 1);
}

void lcd_write_string( char x, char y,char *s)
{
  SELECT();
  lcd_set_xy( x,y );
  while (*s) 
  {
    lcd_write_char(*s);
    s++;
  }
  UNSELECT();
} 

void lcd_clear_line( char y )
{
  char n;
  SELECT();
  lcd_set_xy( 0, y );
  n = 84;
  while( n-- )
  { 
    lcd_write_byte(0x00,1);	 						
  }
  UNSELECT();
} 

void lcd_write_pixels( uint8_t data )
{
   SELECT();
   lcd_write_byte( data, 1 );
   UNSELECT();
}

void lcd_xy( char x,  char y)
{
  SELECT();
  lcd_write_byte(0x40 | y, 0);// column
  lcd_write_byte(0x80 | x, 0);// row
  UNSELECT();
} 

