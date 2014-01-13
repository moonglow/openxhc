/*********************************************************************\
 * Copyright (C) 2014 Roman 'Fallout' Ilichev <fxdteam@gmail.com>    *
 *                                                                   *
 * This file is part of OpenXHC project                              *
 *                             WTFPL LICENSE v2                      *
\*********************************************************************/

#include "stm32f10x.h"
#include "io_macro.h"
#include "kbd_driver.h"

/* COLs input PU */
#define COL1 B, 5
#define COL2 B, 6
#define COL3 B, 7
#define COL4 B, 8
#define COL5 B, 9

/* ROWs output OD */
#define ROW1 B, 12, SPEED_10MHz
#define ROW2 B, 13, SPEED_10MHz
#define ROW3 B, 14, SPEED_10MHz
#define ROW4 B, 15, SPEED_10MHz

/* 
  4x5, but my HW is cheap 4x4 kbd 
  you can found key codes in xhc_mpg_hid_format.pdf
*/
static const uint8_t kbd_key_codes[] = 
{ 
  /* reset, stop, m1, m2 */
  0x17, 0x16,   0x0A,   0x0B,
  /*GoZero, s/p, rewind, Probe-Z */
  0x01, 0x02,   0x03,   0x04, 
  /* Spind, =1/2, =0, Safe-Z */
  0x0C, 0x06,   0x07,   0x08, 
  /* Home, Step+, MPG mode, M3 */ 
  0x09, 0x0E,   0x0D,   0x05,
  /* not used */
  0x00, 0x00,   0x00,   0x00,
};

static uint8_t read_col( uint8_t ncol )
{
  switch( ncol )
  {
    case 0u:
      return PIN_STAT( COL1 );
    case 1u:
      return PIN_STAT( COL2 );
    case 2u:
      return PIN_STAT( COL3 );
    case 3u:
      return PIN_STAT( COL4 );
    case 4u:
      return PIN_STAT( COL5 );
  }
  return 0;
}

#if 0
static uint8_t read_row( uint8_t nrow )
{
  switch( nrow )
  {
    case 0u:
      return PIN_STAT( ROW1 );
    case 1u:
      return PIN_STAT( ROW2 );
    case 2u:
      return PIN_STAT( ROW3 );
    case 3u:
      return PIN_STAT( ROW4 );
  }
  return 0;
}
#endif

static void set_row( uint8_t nrow )
{
  switch( nrow )
  {
    case 0u:
      PIN_HI( ROW1 );
      break;
    case 1u:
      PIN_HI( ROW2 );
      break;
    case 2u:
      PIN_HI( ROW3 );
      break;
    case 3u:
      PIN_HI( ROW4 );
      break;
  }
}

static void reset_row( uint8_t nrow )
{
  switch( nrow )
  {
    case 0u:
      PIN_LOW( ROW1 );
      break;
    case 1u:
      PIN_LOW( ROW2 );
      break;
    case 2u:
      PIN_LOW( ROW3 );
      break;
    case 3u:
      PIN_LOW( ROW4 );
      break;
  }
}

/* return num of keys pressed */
static uint8_t kbd_read( uint8_t *c1, uint8_t *c2 )
{
    uint8_t col, row;
    
    if( c1 == 0 )
      return 0;
    *c1 = 0;
    if( c2 )
      *c2 = 0;
    
    row = 4;
    while( row-- )
    {
      reset_row( row );
      col=32;
      while( --col ) asm("nop");
      col = 5;
      while( col-- )
      {
        uint8_t key;
        if( read_col( col ) )
          continue;
        key = kbd_key_codes[(row<<2)+col];
        if( 0 == *c1 )
        {
          *c1 = key;
        }
        else if( c2 )
        {
          *c2 = key;
          /* dirtyyyy */
          set_row( row );
          return 2;
        }
        else
        {
          set_row( row );
          return 1;
        }
      }
      set_row( row );
    }
    if( *c1 )
      return 1;
    return 0;
}

static void driver_init( void )
{
    /* do for each pin, to allow user change any pin he want */
    PORT_ENABLE_CLOCK( COL1 );
    PORT_ENABLE_CLOCK( COL2 );
    PORT_ENABLE_CLOCK( COL3 );
    PORT_ENABLE_CLOCK( COL4 );
    PORT_ENABLE_CLOCK( COL5 );
    
    PORT_ENABLE_CLOCK( ROW1 );
    PORT_ENABLE_CLOCK( ROW2 );
    PORT_ENABLE_CLOCK( ROW3 );
    PORT_ENABLE_CLOCK( ROW4 );
  
    PIN_INPUT_PU( COL1 );
    PIN_INPUT_PU( COL2 );
    PIN_INPUT_PU( COL3 );
    PIN_INPUT_PU( COL4 );
    PIN_INPUT_PU( COL5 );
    
    /* go HiZ */
    PIN_HI( ROW1 );
    PIN_HI( ROW2 );
    PIN_HI( ROW3 );
    PIN_HI( ROW4 );
    
    PIN_OUT_OD( ROW1 );
    PIN_OUT_OD( ROW2 );
    PIN_OUT_OD( ROW3 );
    PIN_OUT_OD( ROW4 );
}

static void driver_exit( void )
{
  /* do not disable clock, it can be used by other hw */
  PIN_INPUT_FLOATING( COL1 );
  PIN_INPUT_FLOATING( COL2 );
  PIN_INPUT_FLOATING( COL3 );
  PIN_INPUT_FLOATING( COL4 );
  PIN_INPUT_FLOATING( COL5 );
    
  PIN_INPUT_FLOATING( ROW1 );
  PIN_INPUT_FLOATING( ROW2 );
  PIN_INPUT_FLOATING( ROW3 );
  PIN_INPUT_FLOATING( ROW4 );
}

const struct t_kbd_driver kbd_driver = { .init = driver_init, .exit = driver_exit, .read = kbd_read };

