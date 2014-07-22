/*********************************************************************\
* Copyleft (>) 2014 Roman 'Fallout' Ilichev <fxdteam@gmail.com>      *
 *                                                                   *
 * This file is part of OpenXHC project                              *
 *                             WTFPL LICENSE v2                      *
\*********************************************************************/

#pragma once

struct t_lcd_driver
{
  void (*init)(void);
  void (*exit)(void);
  void (*render_screen)( void *p, uint8_t mode, uint8_t mode_ex );
  void (*clear_screen)( void );
  void (*draw_bmp)( void *,int,int,int,int );
  void (*draw_text)( char *,int,int );
};

extern const struct t_lcd_driver lcd_driver;
