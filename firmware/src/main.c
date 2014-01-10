
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_pwr.h"

#include "xhc_dev.h"
#include "nokia_lcd.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

__IO uint8_t PrevXferComplete = 1;
__IO uint8_t HwType = DEV_WHB04;
__IO uint8_t day = 0;
__IO int16_t encoder_val = 0;

void whb03_out( struct whb03_out_data *out );
void whb04_out( struct whb04_out_data *out );


/* EP1 packet goes out OK */
void EP1_IN_Callback(void)
{
  PrevXferComplete = 1; 
}

void xhc_send( void )
{
  struct whb0x_in_data xhc_report;
  
  memset( &xhc_report, 0x00, sizeof ( struct whb0x_in_data ) );
  xhc_report.id = 0x04;
  xhc_report.xor_day = day ^ xhc_report.btn_1;
  xhc_report.wheel = encoder_val;
  xhc_report.wheel_mode = 0x11; /* X stage */
    
  /* Reset the control token to inform upper layer that a transfer is ongoing */
  PrevXferComplete = 0;
  
  /* copy buffer in ENDP1 Tx Packet Memory Area*/
  USB_SIL_Write( EP1_IN, (void*)&xhc_report, sizeof ( struct whb0x_in_data )  );
  
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
  if( HwType == DEV_WHB03 )
  {
    whb03_out( (struct whb03_out_data*)tmp_buff );
  }
  else
  {
    whb04_out( (struct whb04_out_data*)tmp_buff );
  }

}

static const char axis_name[] = "XYZXYZ";
static const char pref_name[] = "WWWMMM";

void whb03_out( struct whb03_out_data *out )
{
  char tmp[16];
  char *s = &tmp[5];
  char i;
  
  tmp[2] = ':';
  tmp[3] = ' ';
   
  day = out->day;
  for( i = 0; i < 6; i++ )
  {
    tmp[0] = pref_name[i];
    tmp[1] = axis_name[i];
    tmp[11]=tmp[12]=tmp[13]=tmp[14]='0';
    sprintf( s, "%.5d.%.2d", out->pos[i].p_int, out->pos[i].p_frac&(~0x80u) );
    tmp[4] = (out->pos[i].p_frac&0x80u)?'-':'+';
    lcd_write_string( 0, i, tmp );
  }
  
}

void whb04_out( struct whb04_out_data *out )
{
  char tmp[16];
  char *s = &tmp[5];
  char i;
  
  tmp[2] = ':';
  tmp[3] = ' ';

  day = out->day;
  for( i = 0; i < 6; i++ )
  {
    tmp[0] = pref_name[i];
    tmp[1] = axis_name[i];
    sprintf( s, "%.5d.%.4d", out->pos[i].p_int, out->pos[i].p_frac&(~0x8000u) );
    tmp[4] = (out->pos[i].p_frac&0x8000u)?'-':'+';
    lcd_write_string( 0, i, tmp );
  }
  
}

__IO uint32_t SwTim1 = 0;
void TIM4_IRQHandler( void )
{
  TIM4->SR = (uint16_t)(~TIM_IT_Update);
  if( SwTim1 )
    --SwTim1;
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

void CheckEmulatedVersion( void )
{
  uint8_t i = 200;
  
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE);
  

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init( GPIOB, &GPIO_InitStructure );
  
  while( --i )
  {
    asm("nop");
    asm("nop");
    asm("nop");
    asm("nop");
  }
  
  HwType = ( GPIOB->IDR & GPIO_Pin_2 )? DEV_WHB04: DEV_WHB03;
}

void Encoder_Config( void )
{
  GPIO_InitTypeDef   GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_ICInitTypeDef  TIM_ICInitStructure;
   
  /* PA0 - A, PA1 - B */
  RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM2, ENABLE );
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
  GPIO_Init( GPIOA, &GPIO_InitStructure);
  
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



int main(void)
{
  Set_System();
  
  /* check XHC verion based on BOOT1 ( PB2 state ) */
  CheckEmulatedVersion();
  Encoder_Config();
  
  
  USB_Interrupts_Config();
  
  Set_USBClock();
  
  USB_Init();
  
  init_delay_timer();
  nokia_hw_init();
  nokia_lcd_init();
  lcd_clear();
  
  if( HwType == DEV_WHB03 )
  {
    lcd_write_string( 0, 0, "XHC HB03" );
  }
  else
  {
    lcd_write_string( 0, 0, "XHC HB04" );
  }
  
  SwTim1 = 2000;
  while (1)
  {
    if (bDeviceState != CONFIGURED )
      continue;
    
    
    if( !SwTim1 )
    {   
        encoder_val = TIM2->CNT;
        TIM2->CNT = 0;
        SwTim1 = 100;
        if( PrevXferComplete )
        {
          xhc_send();
        }
    }
  }
}



