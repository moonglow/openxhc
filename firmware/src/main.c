/*********************************************************************\
* Copyleft (>) 2014 Roman 'Fallout' Ilichev <fxdteam@gmail.com>      *
 *                                                                   *
 * This file is part of OpenXHC project                              *
 *                             WTFPL LICENSE v2                      *
\*********************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "hw_config.h"
#include "usb_lib.h"
#include "usb_pwr.h"

#include "xhc_dev.h"
#include "nokia_lcd.h"
#include "kbd_driver.h"
#include "io_input.h"

__IO uint8_t PrevXferComplete = 1;
__IO uint8_t HwType = DEV_WHB04;
/* used as key for XOR */
__IO uint8_t day = 0;
int16_t encoder_val = 0;
uint8_t rotary_pos = 0;
int g_render_lcd = 0;

struct whb04_out_data output_report = { 0 };
struct whb0x_in_data in_report = { .id = 0x04 };
void whb04_out( struct whb04_out_data *out );

/* convert step multiplier */
static uint16_t mul2val[] = { 0, 1, 5, 10, 20, 30, 40, 50, 100, 500, 1000, 0, 0, 0, 0, 0 };

/* EP1 packet goes out OK */
void EP1_IN_Callback(void)
{
  PrevXferComplete = 1; 
}

void xhc_send( void )
{
  
  
  memset( &in_report, 0x00, sizeof ( struct whb0x_in_data ) );
  in_report.id = 0x04;
  
  kbd_driver.read( &in_report.btn_1, &in_report.btn_2 );
  
  in_report.xor_day = day ^ in_report.btn_1;
  in_report.wheel = encoder_val;
  in_report.wheel_mode = rotary_pos;
    
  /* Reset the control token to inform upper layer that a transfer is ongoing */
  PrevXferComplete = 0;
  
  /* copy buffer in ENDP1 Tx Packet Memory Area*/
  USB_SIL_Write( EP1_IN, (void*)&in_report, sizeof ( struct whb0x_in_data )  );
  
  /* enable endpoint for transmission */
  SetEPTxValid(ENDP1);
}

/* recive state machine */
void xhc_recv( uint8_t *data )
{
#define TMP_BUFF_SIZE   42
#define CHUNK_SIZE      7
  static int offset = 0;
  static uint8_t magic_found = 0;
  static uint8_t tmp_buff[TMP_BUFF_SIZE];
  
  /* check for magic */
  if( *(uint16_t*)data == WHBxx_MAGIC )
  {
      offset = 0;
      magic_found = 1;
  }
  
  if( !magic_found )
    return;
  
  if( (offset+CHUNK_SIZE) > TMP_BUFF_SIZE )
    return;
  memcpy(  &tmp_buff[offset], data, CHUNK_SIZE );
  offset+=CHUNK_SIZE;
  
  /* we recive all data ? */
  if( offset < HwType )
    return;

  magic_found = 0;
  /* convert data to whb04 format */
  if( HwType == DEV_WHB03 )
  {
    struct whb03_out_data* p = (struct whb03_out_data*)tmp_buff;
    int i;
    
    output_report.magic = p->magic;
    output_report.day = p->day;
    output_report.feedrate_ovr = p->feedrate_ovr;
    output_report.sspeed_ovr = p->sspeed_ovr;
    output_report.feedrate = p->feedrate;
    output_report.sspeed = p->sspeed;
    output_report.step_mul = p->step_mul;
    output_report.state = p->state;
    
    for( i = 0; i < 6; i++ )
    {
      uint16_t tmp = (p->pos[i].p_frac & 0x80)<<16;
      p->pos[i].p_frac &=~0x80;
      tmp |= p->pos[i].p_frac * 100l; 
      output_report.pos[i].p_int = p->pos[i].p_int;
      output_report.pos[i].p_frac = tmp;
    }
  }
  else
  {
    output_report = *((struct whb04_out_data*)tmp_buff);
  }
  
  g_render_lcd = 1;
}

static char axis_name[] = "XYZ";
static const char pref_name[] = "WWWMMM";

void whb04_out( struct whb04_out_data *out )
{
  char tmp[18];
  char *s = &tmp[5];
  char i, n;
 
  /* update XOR key */
  day = out->day;
  
  lcd_clear_line( 0 );
  sprintf( tmp, "P:%.2Xh %d*1x", rotary_pos, mul2val[out->step_mul&0x0F] );
  lcd_write_string( 0, 0, tmp );
  sprintf( tmp, "S:  %.5d  %.5d", out->sspeed, out->sspeed_ovr );
  lcd_write_string( 0, 1, tmp );
  sprintf( tmp, "F:  %.5d  %.5d", out->feedrate, out->feedrate_ovr );
  lcd_write_string( 0, 2, tmp );
  
  if( (HwType == DEV_WHB04) && (rotary_pos == 0x18 ) )
    axis_name[0] = 'A';
  else
    axis_name[0] = 'X';

  
  tmp[2] = ':';
  tmp[3] = ' ';
  
  n = io_driver.pos_is_wc()? 0: 3;
  for( i = n; i < (3+n); i++ )
  {
    tmp[0] = pref_name[i];
    tmp[1] = axis_name[i%3];
    sprintf( s, "%.5d.%.4d", out->pos[i].p_int, out->pos[i].p_frac&(~0x8000u) );
    tmp[4] = (out->pos[i].p_frac&0x8000u)?'-':'+';
    lcd_write_string( 0, i+3-n, tmp );
  }
  
}

__IO uint32_t io_poll_tmr = 0;
void TIM4_IRQHandler( void )
{
  TIM4->SR = (uint16_t)(~TIM_IT_Update);
  if( io_poll_tmr )
    --io_poll_tmr;
}

void init_delay_timer( void )
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  
  RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM4, ENABLE );  
  
  /*  72000000/72/1000 = 1000hz */
  TIM_TimeBaseStructure.TIM_Prescaler = SystemCoreClock/1000000ul;
  TIM_TimeBaseStructure.TIM_Period = (1000-1);
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
 
  TIM_TimeBaseInit( TIM4, &TIM_TimeBaseStructure);
  
  TIM4->CNT = 0;
  
  NVIC_EnableIRQ( TIM4_IRQn );
  TIM_ClearITPendingBit( TIM4, TIM_IT_Update );
  TIM_ITConfig( TIM4, TIM_IT_Update, ENABLE);
  
  TIM_Cmd( TIM4, ENABLE);
}

int main(void)
{
  int state_changed = 0;
  
  Set_System();
  /* init encoder, btns, switch etc */
  io_driver.init();
  HwType = io_driver.hw_is_xhb04() ? DEV_WHB04:DEV_WHB03;
  
  /* init usb stuff */
  USB_Interrupts_Config();
  Set_USBClock();
  USB_Init();
  
  /* init matrix keyboard */
  kbd_driver.init();
  
  init_delay_timer();
  nokia_hw_init();
  nokia_lcd_init();
  lcd_clear();
  
  lcd_write_string( 0, 0, (HwType == DEV_WHB03) ? "XHC HB03":"XHC HB04" );

  
  io_poll_tmr = 500;
  for (;;)
  {
    static uint8_t nkeys = 0, c1, c2;
    static int16_t encoder = 0;
    
    if (bDeviceState != CONFIGURED )
      continue;
    
    if( g_render_lcd )
    {
      --g_render_lcd;
      whb04_out( &output_report );
    }
    
    /* update input events state */
    if( !io_poll_tmr )
    {
      uint8_t tmp;
      
      tmp = io_driver.rotary_read();
      if( rotary_pos != tmp )
      {
        state_changed |= 2;
        rotary_pos = tmp;
      }
      
      encoder_val = io_driver.encoder_read();
      if( rotary_pos && (encoder_val || (encoder_val != encoder )) )
      {
          if( encoder_val > 0 )
            encoder_val = 1;
          else if( encoder_val < 0 )
            encoder_val = -1;
          state_changed |= 1;
          encoder = encoder_val;
      }
      
      uint8_t k1, k2;
      do
      {
        k1 = c1, k2 = c2;
        tmp = kbd_driver.read( &c1, &c2 );
      }while( (k1!=c1) || (k2 != c2 ) );
      if( nkeys != tmp )
      {
        state_changed |=4;
        nkeys = tmp;
      }
      io_poll_tmr = 50;
    }

    if( state_changed )
    {   
       xhc_send();
       state_changed = 0;
    }
  }
}



