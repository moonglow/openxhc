/*********************************************************************\
* Copyleft (>) 2014 Roman 'Fallout' Ilichev <fxdteam@gmail.com>      *
 *                                                                   *
 * This file is part of OpenXHC project                              *
 *                             WTFPL LICENSE v2                      *
\*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "stm32f10x.h"
#include "spi_master.h"
#include "io_macro.h"
#include "xhc_dev.h"
#include "timer_drv.h"
#include "lcd_driver.h"

#include "font5x8.c"
/* 
  EastRising 128x64 COG LCD ( ERC12864-4 ) 
  DRIVER: ST7565R
  MODE: 4 wires SPI ( SPI_MODE_3 )
*/

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

uint8_t Contrast_level = 0x16;
#define DISPLAY_ON()           Write_Instruction(0xaf)   //  Display on
#define DISPLAY_OFF()          Write_Instruction(0xae)   //  Display off
#define SET_ADC()              Write_Instruction(0xa1)   //  Reverse disrect (SEG131-SEG0)
#define CLEAR_ADC()            Write_Instruction(0xa0)   //  Normal disrect (SEG0-SEG131)
#define REVERSE_DISPLAY_ON()   Write_Instruction(0xa7)   //  Reverse display : 0 illuminated
#define REVERSE_DISPLAY_OFF()  Write_Instruction(0xa6)   //  Normal display : 1 illuminated
#define ENTIRE_DISPLAY_ON()    Write_Instruction(0xa5)   //  Entire dislay   Force whole LCD point
#define ENTIRE_DISPLAY_OFF()   Write_Instruction(0xa4)   //  Normal display
#define SET_BIAS()             Write_Instruction(0xa3)   //  bias 1
#define CLEAR_BIAS()           Write_Instruction(0xa2)   //  bias 0
#define SET_MODIFY_READ()      Write_Instruction(0xe0)   //  Stop automatic increment of the column address by the read instruction 
#define RESET_MODIFY_READ()    Write_Instruction(0xee)   //  Cancel Modify_read, column address return to its initial value just before the Set Modify Read instruction is started
#define RESET()                Write_Instruction(0xe2)
#define SET_SHL()              Write_Instruction(0xc8)   // SHL 1,COM63-COM0
#define CLEAR_SHL()            Write_Instruction(0xc0)   // SHL 0,COM0-COM63

#define Start_column	0x00
#define Start_page      0x00
#define	StartLine_set	0x00

#define LCD_RESET       A, 2, SPEED_50MHz
#define LCD_CS          A, 4, SPEED_50MHz
#define LCD_DC          A, 3, SPEED_50MHz

#define SELECT()        PIN_LOW( LCD_CS )
#define UNSELECT()      PIN_HI( LCD_CS )

#define LCD_RES_ON()    PIN_LOW( LCD_RESET )
#define LCD_RES_OFF()   PIN_HI( LCD_RESET )

static void Write_Data( uint8_t dat )
{
  PIN_HI( LCD_DC );
  SELECT();
  spi1_send_byte( dat );
  UNSELECT();
}

static void Write_Instruction( uint8_t cmd )
{
  PIN_LOW( LCD_DC );
  SELECT();
  spi1_send_byte( cmd );
  UNSELECT();
}

//Specify DDRAM line for COM0 0~63
static void Initial_Dispay_Line( uint8_t line )
{
  line|=0x40;
  Write_Instruction( line );
}

// Set page address 0~15
static void Set_Page_Address( uint8_t add )
{
  add=0xb0|add;
  Write_Instruction(add);
}

static void Set_Column_Address( uint8_t add )
{
  Write_Instruction((0x10|(add>>4)));
  Write_Instruction((0x0f&add));
}

/* Power_Control   4 (internal converte ON) + 2 (internal regulor ON) + 1 (internal follower ON) */
static void Power_Control( uint8_t vol)
{
  Write_Instruction((0x28|vol));
}

/*  Regulor resistor select
**            1+Rb/Ra  Vo=(1+Rb/Ra)Vev    Vev=(1-(63-a)/162)Vref   2.1v
**            0  3.0       4  5.0(default)
**            1  3.5       5  5.5
**            2  4         6  6
**            3  4.5       7  6.4
*/
static void Regulor_Resistor_Select( uint8_t r)
{
  Write_Instruction((0x20|r));
}

/* a(0-63) 32default   Vev=(1-(63-a)/162)Vref   2.1v */
static void Set_Contrast_Control_Register( uint8_t mod )
{
  Write_Instruction(0x81);
  Write_Instruction(mod);
}

static void Display_Char( char c)
{
  char line, n = 5;
  c-= 32;
  for (line=0; line<n; line++)
  {
    Write_Data( font5x8[c][line] );
  }
}

static void Clear_Line( char y )
{
  uint8_t x = 128;
  Set_Page_Address( y );
  Set_Column_Address( 0x00 );
  
  while( x-- )
  {
    Write_Data( 0x00 );
  }
}

static void erc12864_lcd_init( void )
{
  uint8_t i,j;

  RESET();
  LCD_RES_OFF();
  tmr_v_delay = 250;
  while( tmr_v_delay );
  LCD_RES_ON();
  tmr_v_delay = 800;
  while( tmr_v_delay );
  LCD_RES_OFF();
  tmr_v_delay = 500;
  while( tmr_v_delay );

  CLEAR_ADC();
  SET_SHL();
  CLEAR_BIAS();
  Power_Control(0x07);
  Regulor_Resistor_Select(0x05);
  Set_Contrast_Control_Register(Contrast_level);
  Initial_Dispay_Line(0x00);
  DISPLAY_ON();

  tmr_v_delay = 250;
  while( tmr_v_delay );
  REVERSE_DISPLAY_OFF();
  ENTIRE_DISPLAY_OFF();
  
  /* clear display mem */
  Initial_Dispay_Line(0x40);
  for(i=0;i<0x08;i++)
  {
    Set_Page_Address(i);
    Set_Column_Address(0x00);
    for(j=0;j<0x80;j++)
    {
      Write_Data( 0x00 );
    }
  }
}

static void hw_lcd_init( void )
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
  
  erc12864_lcd_init();
}

static char axis_name[] = "XYZ";
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
static void lcd_render_screen( void *p, uint8_t mode, uint8_t mode_ex )
{
  char tmp[32];
  static char only_once = 1;
  char i;
  struct whb04_out_data *out = (struct whb04_out_data *)p;
  char *s = &tmp[4];
  
  lcd_driver.draw_text( "STATUS: TODO", 0, 0 );
  
  sprintf( tmp, "ROT POSITION:%c", mode2char( mode ) );
  lcd_driver.draw_text( tmp, 0, 1 );
  sprintf( tmp, "MPG:%.4d", mul2val[out->step_mul&0x0F] );
  lcd_driver.draw_text( tmp, 88, 1 ); 
  
  sprintf( tmp, "S: %.5d  F: %.5d", out->sspeed, out->feedrate );
  lcd_driver.draw_text( tmp, 0, 2 );
  
  if( (mode == 0x14) || (mode == 0x15) )
  {
    sprintf( tmp, "O: %.5d  O: %.5d", out->sspeed_ovr, out->feedrate_ovr );
    lcd_driver.draw_text( tmp, 0, 3 );
  }
  else
  {
    Clear_Line( 3 );
  }
  
  if( (g_hw_type == DEV_WHB04) && (mode == 0x18 ) )
    axis_name[0] = 'A';
  else
    axis_name[0] = 'X';
  
  if( only_once )
  {
    lcd_driver.draw_text( "WC", 40, 4 );
    lcd_driver.draw_text( "MC", 98, 4 );
    only_once = 0;
  }
  tmp[1] = ':';
  tmp[2] = ' ';
  for( i = 0; i < 3; i++ )
  {
    tmp[0] = axis_name[i];
    sprintf( s, "%.5d.%.4d  %.5d.%.4d", 
                        out->pos[i].p_int, out->pos[i].p_frac&(~0x8000u),
                        out->pos[i+3].p_int, out->pos[i+3].p_frac&(~0x8000u)
                        );
    tmp[3] = (out->pos[i].p_frac&0x8000u)?'-':'+';
    tmp[15] = (out->pos[i+3].p_frac&0x8000u)?'-':'+';
    lcd_driver.draw_text( tmp, 0, i+5 );
  }
}

static void lcd_clear(void)
{
  uint8_t i, j;
  
  for(i=0;i<0x08;i++)
  {
    Set_Page_Address(i);
    Set_Column_Address(0x00);
    for(j=0;j<0x80;j++)
    {
      Write_Data( 0x00 );
    }
  }
}

static void lcd_write_string( char *s, int x, int y )
{
  Set_Page_Address( y );
  Set_Column_Address( x );
  while (*s) 
  {
    Display_Char(*s);
    s++;
  }
}

const struct t_lcd_driver lcd_driver = 
{
  .init = hw_lcd_init,
  .exit = 0,
  .render_screen = lcd_render_screen,
  .clear_screen = lcd_clear,
  .draw_bmp = 0,
  .draw_text = lcd_write_string,
};
