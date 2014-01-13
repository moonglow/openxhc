/*********************************************************************\
 * Copyright (C) 2014 Roman 'Fallout' Ilichev <fxdteam@gmail.com>    *
 *                                                                   *
 * This file is part of OpenXHC project                              *
 *                             WTFPL LICENSE v2                      *
 * all simple io inputs rotary switch, buttons, encoder              *
\*********************************************************************/

#include "stm32f10x.h"
#include "io_macro.h"
#include "io_input.h"


/* 
  xx - 0x00 - position switch to off state
  P1 - 0x11 - position switch to X state
  P2 - 0x12 - position switch to Y state
  P3 - 0x13 - position switch to Z state
  P4 - 0x15 - position switch to Spindle speed state
  P5 - 0x14 - position switch to Feedrate speed state
  P6 - 0x18 - position switch to Processing speed state ( A-axis for HB04 ) 
*/

#define ROTARY_POS1     A, 8
#define ROTARY_POS2     A, 9
#define ROTARY_POS3     A, 10
#define ROTARY_POS4     B, 10
#define ROTARY_POS5     B, 11
#define ROTARY_POS6     B, 1

/* if you change it you need to rewrite init code too */
#define ENCODER_A       A, 0
#define ENCODER_B       A, 1

/* BOOT1 by default */
#define SELECT_HW       B, 2

/* POSITION STATUS WC or MC */
#define SELECT_POS      C, 13

/* it is not so flexible, but no need to change i thing */
static void encoder_init( void )
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_ICInitTypeDef  TIM_ICInitStructure;
   
  RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2, ENABLE );
  
  /* always on same port */
  PORT_ENABLE_CLOCK( ENCODER_A );
  PIN_INPUT_PU( ENCODER_A );
  PIN_INPUT_PU( ENCODER_B );

  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up | TIM_CounterMode_Down;
  
  TIM_TimeBaseInit( TIM2, &TIM_TimeBaseStructure);
  
  TIM_EncoderInterfaceConfig( TIM2, TIM_EncoderMode_TI12, TIM_ICPolarity_Rising, TIM_ICPolarity_Rising );
  
  TIM_ICStructInit(&TIM_ICInitStructure);
  TIM_ICInitStructure.TIM_ICFilter = 0x04;
  
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
  TIM_ICInit( TIM2, &TIM_ICInitStructure);
  
  TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
  TIM_ICInit( TIM2, &TIM_ICInitStructure);
  
  TIM2->CNT = 0;
  
  TIM_Cmd( TIM2, ENABLE);
}


static int16_t encoder_read( void )
{
  int16_t tmp = TIM2->CNT;
  TIM2->CNT = 0;
  return tmp;
}

static uint8_t rotary_switch_read( void )
{
  if( !PIN_STAT( ROTARY_POS1) )
    return 0x11;
  else if( !PIN_STAT( ROTARY_POS2))
    return 0x12;
  else if( !PIN_STAT( ROTARY_POS3) )
    return 0x13;  
  else if( !PIN_STAT( ROTARY_POS4) )
    return 0x15;
  else if( !PIN_STAT( ROTARY_POS5) )
    return 0x14;
  else if( !PIN_STAT( ROTARY_POS6) )
    return 0x18;    
  return 0;
}

static uint8_t select_hw_read( void )
{
  return PIN_STAT( SELECT_HW );
}

static uint8_t select_pos_read( void )
{
  return PIN_STAT( SELECT_POS );
}

static void io_input_init(  void )
{
  uint8_t t = 255;
  
  /* init rotary switch */
  PORT_ENABLE_CLOCK( ROTARY_POS1 );
  PORT_ENABLE_CLOCK( ROTARY_POS2 );
  PORT_ENABLE_CLOCK( ROTARY_POS3 );
  PORT_ENABLE_CLOCK( ROTARY_POS4 );
  PORT_ENABLE_CLOCK( ROTARY_POS5 );
  PORT_ENABLE_CLOCK( ROTARY_POS6 );
  
  PIN_INPUT_PU( ROTARY_POS1 );
  PIN_INPUT_PU( ROTARY_POS2 );
  PIN_INPUT_PU( ROTARY_POS3 );
  PIN_INPUT_PU( ROTARY_POS4 );
  PIN_INPUT_PU( ROTARY_POS5 );
  PIN_INPUT_PU( ROTARY_POS6 );
  
  /* hardware choose */
  PORT_ENABLE_CLOCK( SELECT_HW );
  PIN_INPUT_PU( SELECT_HW );
  
  /* position chooser */
  PORT_ENABLE_CLOCK( SELECT_POS );
  PIN_INPUT_PD( SELECT_POS );
  
  /* quadrature encoder */
  encoder_init();
  while( t-- ) 
  {
    asm("nop");
  }
}

static void io_input_exit( void )
{
  PIN_INPUT_FLOATING( ROTARY_POS1 );
  PIN_INPUT_FLOATING( ROTARY_POS2 );
  PIN_INPUT_FLOATING( ROTARY_POS3 );
  PIN_INPUT_FLOATING( ROTARY_POS4 );
  PIN_INPUT_FLOATING( ROTARY_POS5 );
  PIN_INPUT_FLOATING( ROTARY_POS6 );  
  
  PIN_INPUT_FLOATING( SELECT_HW );
  
  PIN_INPUT_FLOATING( SELECT_POS );
  /* TODO: encoder cleanup code too */
}

const struct t_io_driver io_driver = { 
  .init = io_input_init, 
  .exit = io_input_exit, 
  .encoder_read = encoder_read,
  .rotary_read = rotary_switch_read,
  .hw_is_xhb04 = select_hw_read,
  .pos_is_wc = select_pos_read,
};
