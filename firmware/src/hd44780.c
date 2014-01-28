/*********************************************************************\
* Copyleft (>) 2014 Roman 'Fallout' Ilichev <fxdteam@gmail.com>      *
 *                                                                   *
 * This file is part of OpenXHC project                              *
 *                             WTFPL LICENSE v2                      *
\*********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f10x.h"
#include "io_macro.h"
#include "xhc_dev.h"

#include "string_utils.h"

#include "lcd_driver.h"
/* 
  20x4 & 16x2
  DRIVER: HD44780
  MODE: 4bit, R/W always 0
*/

/* 
  HW connection: 
  PA2 - E
  PA3 - RS
  GND - R/W
  GND - D0, D1, D2, D3
  PA4 - D4
  PA5 - D5
  PA6 - D6
  PA7 - D7
*/


#define LCD_E           A, 2, SPEED_50MHz
#define LCD_RS          A, 3, SPEED_50MHz

#define LCD_D4          A, 4, SPEED_50MHz
#define LCD_D5          A, 5, SPEED_50MHz
#define LCD_D6          A, 6, SPEED_50MHz
#define LCD_D7          A, 7, SPEED_50MHz

/* delay timer */
#define TIMER_DEV       TIM3
#define TIMER_APB       RCC_APB1PeriphClockCmd
#define TIMER_CLOCK     RCC_APB1Periph_TIM3


/* used for uS delays */
static void hw_timer_init( void )
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  
  TIMER_APB( TIMER_CLOCK, ENABLE );  
  
  TIM_DeInit( TIMER_DEV );
  
  /*  xx000000/xx = 1MHz */
  TIM_TimeBaseStructure.TIM_Prescaler = SystemCoreClock/1000000ul;
  TIM_TimeBaseStructure.TIM_Period = 0; /* use ARR as delay */
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
 
  TIM_TimeBaseInit( TIMER_DEV, &TIM_TimeBaseStructure);
  
  TIMER_DEV->CNT = 0;
  /* timer will switch off automaticaly, after counter will equal to ARR */
  TIM_SelectOnePulseMode( TIMER_DEV, TIM_OPMode_Single );
}

static void delay_us( uint16_t us )
{
    /* long wire compensation */
    TIMER_DEV->ARR = (us*4);
    TIMER_DEV->CR1 |= TIM_CR1_CEN;
    while( TIMER_DEV->CR1 & TIM_CR1_CEN );
}


static void lcd_nibble_out( uint8_t n )
{
  if( n & 0x01 )
    PIN_HI( LCD_D4 );
  else
    PIN_LOW( LCD_D4 );

  if( n & 0x02)
    PIN_HI( LCD_D5 );
  else
    PIN_LOW( LCD_D5 );
      
  if( n & 0x04)
    PIN_HI( LCD_D6 );
  else
    PIN_LOW( LCD_D6 );

  if( n & 0x08)
    PIN_HI( LCD_D7 );
  else
    PIN_LOW( LCD_D7 );
  
  PIN_HI( LCD_E );
  delay_us( 10 );
  PIN_LOW( LCD_E );
  delay_us( 50 );
}

static void lcd_write_byte( uint8_t d )
{
  lcd_nibble_out( d>>4 );
  lcd_nibble_out( d );
}

static void lcd_write_cmd( uint8_t c )
{
  PIN_LOW( LCD_RS );
  delay_us( 50 );
  lcd_write_byte( c );
}

static void lcd_write_data( uint8_t d )
{
  PIN_HI( LCD_RS );
  delay_us( 50 );
  lcd_write_byte( d );
}

static void hd44780_hw_init( void )
{
  hw_timer_init();
  
  PORT_ENABLE_CLOCK_START()
      PORT_ENABLE_CLOCK_ENTRY( LCD_E )
      PORT_ENABLE_CLOCK_ENTRY( LCD_RS )
      PORT_ENABLE_CLOCK_ENTRY( LCD_D4 )
      PORT_ENABLE_CLOCK_ENTRY( LCD_D5 )
      PORT_ENABLE_CLOCK_ENTRY( LCD_D6 )
      PORT_ENABLE_CLOCK_ENTRY( LCD_D7 )
  PORT_ENABLE_CLOCK_END()
      
  PIN_LOW( LCD_E );
  PIN_LOW( LCD_RS );
  
  PIN_LOW( LCD_D4 );
  PIN_LOW( LCD_D5 );
  PIN_LOW( LCD_D6 );
  PIN_LOW( LCD_D7 );
     
  PIN_OUT_PP( LCD_E );
  PIN_OUT_PP( LCD_RS );
  
  PIN_OUT_PP( LCD_D4 );
  PIN_OUT_PP( LCD_D5 );
  PIN_OUT_PP( LCD_D6 );
  PIN_OUT_PP( LCD_D7 );

  delay_us( 1000 );
  
  /* 4 bit mode */
  lcd_nibble_out( 0x03 );
  delay_us( 1000 );
  lcd_nibble_out( 0x03 );
  delay_us( 1000 );
  lcd_nibble_out( 0x03 );
  delay_us( 1000 );
  
  /* 4 bit mode */
  lcd_nibble_out( 0x02 );
  delay_us( 50 );
  
  /* function set 5x8 */
  lcd_write_cmd( 0x28 );
  delay_us( 50 );
  
  /* display on/off control */
  lcd_write_cmd( 0x0C );
  delay_us( 50 );
 
  /* entry mode */
  lcd_write_cmd( 0x06 );
  delay_us( 50 );
  
  /* clear screen */
  lcd_write_cmd( 0x01 );
  delay_us( 2000 );
}

static void hd44780_clear(void)
{
    lcd_write_cmd( 0x01 );
    /* about 1.53ms to execute */
    delay_us( 2000 );
}

static const uint8_t row_addr_table[] = { 0x00, 0x40, 0x14, 0x54 };
static void hd44780_write_string( char *s, int x, int y )
{
  lcd_write_cmd( (0x80 | row_addr_table[y]) + x ); 
  delay_us( 50 );
  while ( *s )
  {
    lcd_write_data( *s++ );
  }
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

static void hd44780_render_screen( void *p, uint8_t mode, uint8_t mode_ex )
{
  char tmp[24];
  char i;
  struct whb04_out_data *out = (struct whb04_out_data *)p;
  
  tmp[0] = mode2char( mode );
  tmp[1] = ' ';
  tmp[2] = 'M';
  string2uint( mul2val[out->step_mul&0x0F], 4, &tmp[3] );
  tmp[7] = ' ';
  tmp[8] = ' ';
  tmp[9] = 'S';
  string2uint( out->sspeed, 4, &tmp[10] );
  tmp[14] = ' ';
  tmp[15] = 'F';
  string2uint( out->feedrate, 4, &tmp[16] );

  lcd_driver.draw_text( tmp, 0, 0 );
  
  if( (g_hw_type == DEV_WHB04) && (mode == 0x18 ) )
    axis_name[0] = 'A';
  else
    axis_name[0] = 'X';
  
  if( (mode == 0x14) || (mode == 0x15) )
  {
    strcpy( tmp, "SPEED       FEEDRATE" );
    lcd_driver.draw_text( tmp, 0, 1 );
    memset( tmp, ' ', sizeof( tmp ) );
    
    string2uint( out->sspeed, 5,  &tmp[0] );
    tmp[5] = ' ';
    string2uint( out->feedrate, 5,  &tmp[15] );
    lcd_driver.draw_text( tmp, 0, 2 );
    
    string2uint( out->sspeed_ovr, 5,  &tmp[0] );
    tmp[5] = ' ';
    string2uint( out->feedrate_ovr, 5,  &tmp[15] );
    lcd_driver.draw_text( tmp, 0, 3 );
    return;
  }

  for( i = 0; i < 3; i++ )
  {
    tmp[0] = axis_name[i];
    xhc2string( out->pos[i].p_int, out->pos[i].p_frac, 4, 3, &tmp[1] );
    tmp[10] = ' ';
    xhc2string( out->pos[i+3].p_int, out->pos[i+3].p_frac, 4, 3, &tmp[11] );
    tmp[20] = 0;
    lcd_driver.draw_text( tmp, 0, i+1 );
  }
}

const struct t_lcd_driver lcd_driver = 
{
  .init = hd44780_hw_init,
  .exit = 0,
  .render_screen = hd44780_render_screen,
  .clear_screen = hd44780_clear,
  .draw_bmp = 0,
  .draw_text = hd44780_write_string,
};
