/*********************************************************************\
* Copyleft (>) 2014 Roman 'Fallout' Ilichev <fxdteam@gmail.com>      *
 *                                                                   *
 * This file is part of OpenXHC project                              *
 *                             WTFPL LICENSE v2                      *
\*********************************************************************/

#pragma once

struct t_timer_driver
{
  void (*init)(void);
  void (*exit)(void);
};

extern const struct t_timer_driver timer_driver;
extern __IO uint32_t io_poll_tmr;
extern __IO uint32_t tmr_v_delay;

