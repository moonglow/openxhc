#include <stdlib.h>
#include "stm32f10x.h"
#include "spi_master.h"

#include "nokia_lcd.h"
/* nokia 5100 driver */

#define LCD_MEM_W 84
#define LCD_MEM_H 48
#define LCD_MEM_SIZE ((LCD_MEM_W*LCD_MEM_H)/8)

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


/* 84 x 48 pixels 504 bytes RAM */
static uint8_t lcd_mem[LCD_MEM_SIZE];
extern const uint8_t font5x8[][5];

#define LCD_RESET_PORT  GPIOA
#define LCD_RESET_PIN   GPIO_Pin_2

#define LCD_CS_PORT     GPIOA
#define LCD_CS_PIN      GPIO_Pin_4
    
#define LCD_DC_PORT     GPIOA
#define LCD_DC_PIN      GPIO_Pin_3
    
#define SELECT() LCD_CS_PORT->ODR &= (~LCD_CS_PIN)
#define UNSELECT() LCD_CS_PORT->ODR |= (LCD_CS_PIN)

#define LCD_RES_ON() LCD_RESET_PORT->ODR &= (~LCD_RESET_PIN)
#define LCD_RES_OFF() LCD_RESET_PORT->ODR |= (LCD_RESET_PIN)

void nokia_hw_init( void )
{
  GPIO_InitTypeDef GPIO_InitStructure;
  spi_init( 1 );
  
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE);

  GPIO_InitStructure.GPIO_Pin = LCD_RESET_PIN | LCD_CS_PIN | LCD_DC_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init( GPIOA, &GPIO_InitStructure );
}

void lcd_pwm_bk_percent( uint8_t bkl )
{

}


void lcd_write_byte( uint8_t data, char cmd )
{
  if( cmd )
  {
    LCD_DC_PORT->ODR |= LCD_DC_PIN; 
  }
  else
  {
    LCD_DC_PORT->ODR &= (~LCD_DC_PIN); 
  }
  
  spi1_send_byte( data );
}

void nokia_lcd_init( void )
{
  int i;
  
  LCD_RES_ON();
  i = LCD_MEM_SIZE;
  while( i-- )
  {
    lcd_mem[i] = 0x00;
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
  char t;
  char k;
  SELECT();
  lcd_set_xy(0,0);
  for(t=0;t<6;t++)
  { 
    for(k=0;k<84;k++)
    { 
      lcd_write_byte(0x00,1);	 						
    } 
  }
  UNSELECT();
}

void lcd_clear_z(void)
{
  int i = LCD_MEM_SIZE;
  while( i-- )
  {
    lcd_mem[i] = 0x00;
  } 
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


void lcd_put_pixel( char x, char y, char color )
{
  if (x > (LCD_MEM_W-1) || y > (LCD_MEM_H-1)) 
  {
    return;
  }
  
  uint16_t offset = x+((y>>3)*LCD_MEM_W);
  if (color)
  {
    lcd_mem[offset] |= (1<<(y&7));
  } 
  else 
  {
    lcd_mem[offset] &= ~(1<<(y&7));
  }  
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

void lcd_render( char xs, char ys, char xe, char ye )
{
  int i;
  for( ;ys < ye; ys+=8 )
  {
    lcd_xy( xs, ys>>3 );
    uint16_t offset = xs+((ys>>3)*LCD_MEM_W);
    i = xe - xs;
    if( i < 0 )
      return;
    while( i-- )
    {
      lcd_write_pixels( lcd_mem[offset] );  
      ++offset;
    }
  }
}


void lcd_line( char x0, char y0, char x1, char y1 )
{
  int dx, dy, sx, sy, err, e2;
  dx = abs(x1 - x0);
  dy = abs(y1 - y0);
    
  if( x0 < x1 )
    sx = 1;
  else
    sx = -1;
  
  if( y0 < y1 )
    sy = 1;
  else
    sy = -1;
  
  err = dx-dy;
  
  for ( ;;)
  {
    lcd_put_pixel( x0, y0, 1);
    if( (x0 == x1) && (y0 == y1 ) )
      break;
    e2 = 2*err;
    if( e2 > -dy )
    {
      err = err - dy;
      x0 = x0 + sx;
    }
    if( e2 < dx )
    {
      err = err + dx;
      y0 = y0 + sy;
    }
  }
}

