/*********************************************************************\
* Copyleft (>) 2014 Roman 'Fallout' Ilichev <fxdteam@gmail.com>      *
 *                                                                   *
 * This file is part of OpenXHC project                              *
 *                             WTFPL LICENSE v2                      *
\*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "spi_master.h"
#include "io_macro.h"
#include "xhc_dev.h"
#include "string_utils.h"
#include "lcd_driver.h"

#include "font5x8.c"
/* nokia 5100 driver */

/* 
  DRIVER: PCD8544
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

#define LCD_RESET       A, 2, SPEED_50MHz
#define LCD_CS          A, 4, SPEED_50MHz
#define LCD_DC          A, 3, SPEED_50MHz

#define SELECT()        PIN_LOW( LCD_CS )
#define UNSELECT()      PIN_HI( LCD_CS )

#define LCD_RES_ON()    PIN_LOW( LCD_RESET )
#define LCD_RES_OFF()   PIN_HI( LCD_RESET )

static void lcd_write_byte( uint8_t data, char cmd )
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

static void lcd_set_xy( char x, char  y )
{
  lcd_write_byte(0x40 | y, 0);// column
  lcd_write_byte(0x80 | x, 0);// row
} 

static void nokia_lcd_init( void )
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

static void nokia_hw_init( void )
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
  
  nokia_lcd_init();
}

static void lcd_clear(void)
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

static void lcd_write_char( char c)
{
  char line, n = 5;
  c-= 32;
  for (line=0; line<n; line++)
  lcd_write_byte(font5x8[c][line], 1);
}

static void lcd_write_string( char *s, int x, int y )
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


static void lcd_clear_line( char y )
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
#if 0
static void lcd_write_pixels( uint8_t data )
{
   SELECT();
   lcd_write_byte( data, 1 );
   UNSELECT();
}

static void lcd_xy( char x,  char y)
{
  SELECT();
  lcd_write_byte(0x40 | y, 0);// column
  lcd_write_byte(0x80 | x, 0);// row
  UNSELECT();
} 
#endif

static char axis_name[] = "XYZ";
static const char pref_name[] = "WWWMMM";
/* convert step multiplier */
static uint16_t mul2val[] = { 0, 1, 5, 10, 20, 30, 40, 50, 100, 500, 1000, 0, 0, 0, 0, 0 };
static char mode2char( uint8_t mode )
{
  switch( mode )
  {
    case 0x11:
      return 'X';
    case 0x12:
      return 'Y';
    case 0x13:
      return 'Z';
    case 0x14:
      return 'S';
    case 0x15:
      return 'F';
    case 0x18:
      if(g_hw_type == DEV_WHB04)
        return 'A';
      else
        return 'T';
  }
  return 'N';
}
/* display data to specific display */
static void nokia_render_screen( void *p, uint8_t mode, uint8_t mode_ex )
{
  char tmp[22];
  char i, n;
  struct whb04_out_data *out = (struct whb04_out_data *)p;
 
  tmp[0] = 'P';
  tmp[1] = ':';
  tmp[2] = mode2char( mode );
  tmp[3] = ' ';
  tmp[4] = 'M';
  tmp[5] = ':';
  string2uint( mul2val[out->step_mul&0x0F], 4, &tmp[6] );
  tmp[10] = '*';
  tmp[11] = 0;
  lcd_driver.draw_text( tmp, 0, 0 );
  
  tmp[0] = 'S';
  tmp[1] = ':';
  string2uint( out->sspeed, 5, &tmp[2] );
  tmp[7] = ' ';
  tmp[8] = ' ';
  tmp[9] = 'F';
  tmp[10] = ':';
  string2uint( out->feedrate, 5, &tmp[11] );
  lcd_driver.draw_text( tmp, 0, 1 );
  
  if( (mode == 0x14) || (mode == 0x15) )
  {
    tmp[0] = '0';
    tmp[1] = ':';
    string2uint( out->sspeed_ovr, 5, &tmp[2] );
    tmp[7] = ' ';
    tmp[8] = ' ';
    tmp[9] = '0';
    tmp[10] = ':';
    string2uint( out->feedrate_ovr, 5, &tmp[11] );
    lcd_driver.draw_text( tmp, 0, 2 );
  }
  else
  {
    lcd_clear_line( 2 );
  }
  
  if( (g_hw_type == DEV_WHB04) && (mode == 0x18 ) )
    axis_name[0] = 'A';
  else
    axis_name[0] = 'X';

  
  tmp[2] = ':';
  tmp[3] = ' ';
  tmp[4] = ' ';
  
  n = ( mode_ex == 1 )? 0: 3;
  for( i = n; i < (3+n); i++ )
  {
    tmp[0] = pref_name[i];
    tmp[1] = axis_name[i%3];
    xhc2string( out->pos[i].p_int, out->pos[i].p_frac, 5, 4, &tmp[5] );
    lcd_driver.draw_text( tmp, 0, i+3-n );
  }
}


const struct t_lcd_driver lcd_driver = 
{
  .init = nokia_hw_init,
  .exit = 0,
  .render_screen = nokia_render_screen,
  .clear_screen = lcd_clear,
  .draw_bmp = 0,
  .draw_text = lcd_write_string,
};