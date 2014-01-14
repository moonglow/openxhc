#pragma once

void nokia_hw_init( void );
void nokia_hw_deinit( void );
void nokia_lcd_init( void );
void lcd_clear(void);


void lcd_write_string( char x, char y,char *s);
void lcd_write_char( char c);

void lcd_clear_line( char y );

