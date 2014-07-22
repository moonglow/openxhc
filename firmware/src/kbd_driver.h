#pragma once


struct t_kbd_driver
{
  void (*init)(void);
  void (*exit)(void);
  uint8_t (*read)(uint8_t*,uint8_t*);
};


extern const struct t_kbd_driver kbd_driver;
