#pragma once

void nokia_hw_init( void );
void nokia_hw_deinit( void );
void nokia_lcd_init( void );
void lcd_clear(void);
void lcd_clear_z(void);

void lcd_write_string( char x, char y,char *s);
void lcd_write_char( char c);

void lcd_put_pixel( char x, char y, char color );
void lcd_line( char x0, char y0, char x1, char y1 );
void lcd_render( char xs, char ys, char xe, char ye );
void lcd_pwm_bk_percent( uint8_t bkl );

