/*********************************************************************\
* Copyleft (>) 2014 Roman 'Fallout' Ilichev <fxdteam@gmail.com>      *
 *                                                                   *
 * This file is part of OpenXHC project                              *
 *                             WTFPL LICENSE v2                      *
\*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "io_macro.h"
#include "timer_drv.h"
#include "spi_master.h"
#include "xhc_dev.h"
#include "string_utils.h"
#include "lcd_driver.h"

#include "st7735_regs.h"
#include "openxhc_logo.c"
#include "font5x8.c"

/*  
  DRIVER: ST7735R
  MODE: 4 wires SPI ( SPI_MODE_0 )
  based on chinese code what comes with module
*/

/* 
  HW connection: 
  PA5 - SCLK
  PA6 - MISO
  PA7 - MOSI
  PA3 - RS
  PA2 - RESET
  PA4 - LCD CS
  GND - BKL
  GND
  VDD
*/

#define LCD_RESET       A, 2, SPEED_50MHz
#define LCD_CS          A, 4, SPEED_50MHz
#define LCD_RS          A, 3, SPEED_50MHz

#define LCD_W 128u
#define LCD_H 160u

/* 565 color */
static uint16_t font_color = 0x07E0;

static void delay_ms( uint16_t ms )
{
  tmr_v_delay = ms;
  while( tmr_v_delay );
}

static void st7735_write_cmd( uint8_t cmd )
{
  PIN_LOW( LCD_RS );
  PIN_LOW( LCD_CS );
  
  spi1_send_byte( cmd );
  
  PIN_HI( LCD_CS );
}

static void st7735_write_data16( uint16_t data )
{
  PIN_HI( LCD_RS );
  PIN_LOW( LCD_CS );
  
  spi1_send_byte( (data>>8)&0xFF );
  spi1_send_byte( (data&0xFF) );

  PIN_HI( LCD_CS );
}

static void st7735_write_data( uint8_t data )
{
  PIN_HI( LCD_RS );
  PIN_LOW( LCD_CS );
  
  spi1_send_byte( data );
  
  PIN_HI( LCD_CS );
}

static void init_st7735r( void )
{
  /* wait a little for reset */
  delay_ms( 500 );
  PIN_HI( LCD_RESET );
  delay_ms( 200 );
  
  /* software reset */
  st7735_write_cmd(ST7735_SWRESET);
  delay_ms(150);

  /* out of sleep mode */
  st7735_write_cmd(ST7735_SLPOUT);
  delay_ms(500);
  /* frame rate control - normal mode */
  st7735_write_cmd(ST7735_FRMCTR1);  
  st7735_write_data(0x01);  /* frame rate = fosc / (1 x 2 + 40) * (LINE + 2C + 2D) */
  st7735_write_data(0x2C); 
  st7735_write_data(0x2D); 
  /* frame rate control - idle mode */
  st7735_write_cmd(ST7735_FRMCTR2);
  st7735_write_data(0x01);  /* frame rate = fosc / (1 x 2 + 40) * (LINE + 2C + 2D) */
  st7735_write_data(0x2C); 
  st7735_write_data(0x2D); 

  /* frame rate control - partial mode */
  st7735_write_cmd(ST7735_FRMCTR3);
  st7735_write_data(0x01); /* dot inversion mode */
  st7735_write_data(0x2C); 
  st7735_write_data(0x2D); 
  st7735_write_data(0x01); /* line inversion mode */
  st7735_write_data(0x2C); 
  st7735_write_data(0x2D); 
  
  /* display inversion control */
  st7735_write_cmd(ST7735_INVCTR); 
  st7735_write_data(0x07);  /* no inversion */

  /* power control */
  st7735_write_cmd(ST7735_PWCTR1);
  st7735_write_data(0xA2);      
  st7735_write_data(0x02);      /* -4.6V */
  st7735_write_data(0x84);      /* AUTO mode */

  /* power control */
  st7735_write_cmd(ST7735_PWCTR2); 
  st7735_write_data(0xC5);      /* VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD */

  /* power control */
  st7735_write_cmd(ST7735_PWCTR3);
  st7735_write_data(0x0A);      /* Opamp current small */
  st7735_write_data(0x00);      /* Boost frequency */

  /* power control */
  st7735_write_cmd(ST7735_PWCTR4);
  st7735_write_data(0x8A);      /* BCLK/2, Opamp current small & Medium low */
  st7735_write_data(0x2A);     

   /* power control */
  st7735_write_cmd(ST7735_PWCTR5);
  st7735_write_data(0x8A);    
  st7735_write_data(0xEE);     

   /* power control */
  st7735_write_cmd(ST7735_VMCTR1);
  st7735_write_data(0x0E);  

  /* don't invert display */
  st7735_write_cmd(ST7735_INVOFF);

  /* memory access control (directions) */
  st7735_write_cmd(ST7735_MADCTL);
  /* row address/col address, bottom to top refresh */
  st7735_write_data(0xC8);
  //madctl = 0xC8;
  
  /* color mode */
  st7735_write_cmd(ST7735_COLMOD);
  st7735_write_data(0x05);   /* 16-bit color */

  /* column addr set */
  st7735_write_cmd(ST7735_CASET);
  st7735_write_data(0x00);
  st7735_write_data(0x00);   /* XSTART = 0 */
  st7735_write_data(0x00);
  st7735_write_data(0x7F);   /* XEND = 127 */

  /* row addr set */
  st7735_write_cmd(ST7735_RASET);
  st7735_write_data(0x00);
  st7735_write_data(0x00);    /* XSTART = 0 */
  st7735_write_data(0x00);
  st7735_write_data(0x9F);    /* XEND = 159 */

  st7735_write_cmd(ST7735_GMCTRP1);
  st7735_write_data(0x0f);
  st7735_write_data(0x1a);
  st7735_write_data(0x0f);
  st7735_write_data(0x18);
  st7735_write_data(0x2f);
  st7735_write_data(0x28);
  st7735_write_data(0x20);
  st7735_write_data(0x22);
  st7735_write_data(0x1f);
  st7735_write_data(0x1b);
  st7735_write_data(0x23);
  st7735_write_data(0x37);
  st7735_write_data(0x00);
  st7735_write_data(0x07);
  st7735_write_data(0x02);
  st7735_write_data(0x10);
  st7735_write_cmd(ST7735_GMCTRN1);
  st7735_write_data(0x0f); 
  st7735_write_data(0x1b); 
  st7735_write_data(0x0f); 
  st7735_write_data(0x17); 
  st7735_write_data(0x33); 
  st7735_write_data(0x2c); 
  st7735_write_data(0x29); 
  st7735_write_data(0x2e); 
  st7735_write_data(0x30); 
  st7735_write_data(0x30); 
  st7735_write_data(0x39); 
  st7735_write_data(0x3f); 
  st7735_write_data(0x00); 
  st7735_write_data(0x07); 
  st7735_write_data(0x03); 
  st7735_write_data(0x10); 
  
  st7735_write_cmd(ST7735_DISPON);
  delay_ms(100);

  /* normal display on */
  st7735_write_cmd(ST7735_NORON);
  delay_ms(10);
}

static void st7735_set_addr_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
  /* column addr set */
  st7735_write_cmd( ST7735_CASET );
  st7735_write_data( 0x00 );
  st7735_write_data( x0 );   /* XSTART */ 
  st7735_write_data( 0x00);
  st7735_write_data( x1 );  /* XEND */

  /* row addr set */
  st7735_write_cmd( ST7735_RASET );
  st7735_write_data( 0x00 );
  st7735_write_data( y0 );    /* YSTART */
  st7735_write_data( 0x00 );
  st7735_write_data( y1 );    /* YEND */

  st7735_write_cmd( ST7735_RAMWR );
}

/* dirty and no needed, but looks nice */
static void st7735_draw_logo( void )
{
  uint16_t logo_size = sizeof( logo_12856_4bit );
  const uint8_t  *logo_ptr = logo_12856_4bit;
  st7735_set_addr_window( 0, 0, 127, 55 );
  while( logo_size-- )
  {
    uint8_t index = *logo_ptr;
    st7735_write_data16( logo_12856_4bit_colors[index>>4] );
    st7735_write_data16( logo_12856_4bit_colors[index&0x0F] );
    ++logo_ptr;
  }
}

static void st7735_hw_init( void )
{
  spi_init_ex( 1, 3000000 );
  
  PORT_ENABLE_CLOCK( LCD_RESET );
  PORT_ENABLE_CLOCK( LCD_CS );
  PORT_ENABLE_CLOCK( LCD_RS );
  
  PIN_LOW( LCD_RESET );
  PIN_HI( LCD_CS );
  
  PIN_OUT_PP( LCD_RESET );
  PIN_OUT_PP( LCD_CS );
  PIN_OUT_PP( LCD_RS );
  
  init_st7735r();
}

static void st7735_write_char( char c, uint8_t x, uint8_t y )
{
  char n, i;
  uint8_t d;
  c-= 32;
  for (n=0; n<5; n++)
  {
    d = font5x8[c][n];
    st7735_set_addr_window( x, y, x, y+7 );
    ++x;
    i = 8;
    while( i-- )
    {
      st7735_write_data16( (d&0x01)?font_color:0 );
      d >>= 1;
    }
  }
}

static void st7735_write_string( char *s, int x, int y )
{
  /* correct column address like other display do */
  y=(y*8)+76;
  while (*s) 
  {
    st7735_write_char( *s, x, y );
    /* for 5 dots font w */
    x+=5;
    s++;
  }
}

static void st7735_clear_line( int y )
{
  uint16_t n = 128*8;
  y=(y*8)+76;
  st7735_set_addr_window( 0, y, 127, y+7 );
  while( n-- )
  {
    st7735_write_data16( 0 );
  }
}

static void st7735_lcd_clear( void )
{
  uint16_t n = LCD_H*LCD_W;
  
  st7735_set_addr_window( 0, 0, LCD_W-1, LCD_H-1 );

   /* setup for data burst */
  PIN_HI( LCD_RS );
  PIN_LOW( LCD_CS );
  
  /* byte order is big endian */
  while( n-- )
  {
    spi1_send_byte( 0 );
    spi1_send_byte( 0 );
  }
  PIN_HI( LCD_CS );
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

static void st7735_render_screen( void *p, uint8_t mode, uint8_t mode_ex )
{
  char tmp[32];
  static char only_once = 1;
  char i;
  struct whb04_out_data *out = (struct whb04_out_data *)p;
  
  lcd_driver.draw_text( "STATUS: TODO", 0, 0 );
  
  sprintf( tmp, "POS: %c  ", mode2char( mode ) );
  lcd_driver.draw_text( tmp, 0, 1 );
  sprintf( tmp, "MPG: " );
  string2uint( mul2val[out->step_mul&0x0F], 4, &tmp[5] );
  lcd_driver.draw_text( tmp, 75, 1 ); 
  
  tmp[0] = 'S';
  tmp[1] = ':';
  tmp[2] = ' ';
  string2uint( out->sspeed, 5, &tmp[3] );
  tmp[8] = ' ';
  tmp[9] = ' ';
  tmp[10] = 'F';
  tmp[11] = ':';
  tmp[12] = ' ';
  string2uint( out->feedrate, 5, &tmp[13] );
  lcd_driver.draw_text( tmp, 0, 2 );
  
  if( (mode == 0x14) || (mode == 0x15) )
  {
    tmp[0] = 'O';
    tmp[1] = ':';
    tmp[2] = ' ';
    string2uint( out->sspeed_ovr, 5, &tmp[3] );
    tmp[8] = ' ';
    tmp[9] = ' ';
    tmp[10] = 'O';
    tmp[11] = ':';
    tmp[12] = ' ';
    string2uint( out->feedrate_ovr, 5, &tmp[13] );
    lcd_driver.draw_text( tmp, 0, 3 );
  }
  else
  {
    st7735_clear_line( 3 );
  }
  
  if( (g_hw_type == DEV_WHB04) && (mode == 0x18 ) )
    axis_name[0] = 'A';
  else
    axis_name[0] = 'X';
  
  if( only_once )
  {
    st7735_draw_logo();
    lcd_driver.draw_text( "WC", 35, 4 );
    lcd_driver.draw_text( "MC", 95, 4 );
    only_once = 0;
  }

  for( i = 0; i < 3; i++ )
  {
    tmp[0] = axis_name[i];
    tmp[1] = ' ';
    xhc2string( out->pos[i].p_int, out->pos[i].p_frac, 5, 4, &tmp[2] );
    tmp[13] = ' ';
    xhc2string( out->pos[i+3].p_int, out->pos[i+3].p_frac, 5, 4, &tmp[14] );
    lcd_driver.draw_text( tmp, 0, i+5 );
  }
}


const struct t_lcd_driver lcd_driver = 
{
  .init = st7735_hw_init,
  .exit = 0,
  .render_screen = st7735_render_screen,
  .clear_screen = st7735_lcd_clear,
  .draw_bmp = 0,
  .draw_text = st7735_write_string,
};
