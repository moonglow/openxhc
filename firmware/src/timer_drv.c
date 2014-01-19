/*********************************************************************\
* Copyleft (>) 2014 Roman 'Fallout' Ilichev <fxdteam@gmail.com>      *
 *                                                                   *
 * This file is part of OpenXHC project                              *
 *                             WTFPL LICENSE v2                      *
\*********************************************************************/

#include "stm32f10x.h"

#include "timer_drv.h"

/*  to make more easy if need change timer */
#define TIMER_DEV       TIM4
#define TIMER_APB       RCC_APB1PeriphClockCmd
#define TIMER_CLOCK     RCC_APB1Periph_TIM4
#define TIMER_IRQn      TIM4_IRQn
#define TIMER_ISR       TIM4_IRQHandler

__IO uint32_t io_poll_tmr = 0;
__IO uint32_t tmr_v_delay = 0;

void TIMER_ISR( void )
{
  TIMER_DEV->SR = (uint16_t)(~TIM_IT_Update);
  if( io_poll_tmr )
    --io_poll_tmr;
  if( tmr_v_delay )
    --tmr_v_delay;
}

/* setup timer resolution to 1ms */
static void init_timer( void )
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  
  TIMER_APB( TIMER_CLOCK, ENABLE );  
  
  /*  xx000000/xx/1000 = 1000hz */
  TIM_TimeBaseStructure.TIM_Prescaler = SystemCoreClock/1000000ul;
  TIM_TimeBaseStructure.TIM_Period = (1000-1);
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
 
  TIM_TimeBaseInit( TIMER_DEV, &TIM_TimeBaseStructure);
  
  TIMER_DEV->CNT = 0;
  
  NVIC_EnableIRQ( TIMER_IRQn );
  TIM_ClearITPendingBit( TIMER_DEV, TIM_IT_Update );
  TIM_ITConfig( TIMER_DEV, TIM_IT_Update, ENABLE);
  
  TIM_Cmd( TIMER_DEV, ENABLE); 
}

static void exit_timer( void )
{
    TIM_Cmd( TIMER_DEV, DISABLE ); 
    NVIC_DisableIRQ( TIMER_IRQn );
    TIMER_APB( TIMER_CLOCK, DISABLE );  
}

const struct t_timer_driver timer_driver = { .init = init_timer, .exit = exit_timer };
