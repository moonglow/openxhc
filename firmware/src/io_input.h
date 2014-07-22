#pragma once

struct t_io_driver
{
  void (*init)(void);
  void (*exit)(void);
  int16_t (*encoder_read)( void );
  uint8_t (*rotary_read)( void );
  uint8_t (*hw_is_xhb04)( void );
  uint8_t (*pos_is_wc)( void );
};


extern const struct t_io_driver io_driver;
