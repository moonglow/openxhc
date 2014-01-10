
#include "hw_config.h"
#include "usb_lib.h"
#include "usb_pwr.h"



EXTI_InitTypeDef EXTI_InitStructure;


extern __IO uint8_t PrevXferComplete;


void Set_System(void)
{
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);


  GPIO_AINConfig();

  /* Configure the EXTI line 18 connected internally to the USB IP ************/
  EXTI_ClearITPendingBit(EXTI_Line18);
  EXTI_InitStructure.EXTI_Line = EXTI_Line18;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
}



void Set_USBClock(void)
{
  if( SystemCoreClock == 72000000ul )
  {
    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_1Div5);
  }
  else if( SystemCoreClock == 48000000ul )
  {
    RCC_USBCLKConfig(RCC_USBCLKSource_PLLCLK_Div1);
  }
  else
  {
    /* unsupported clock */
    for(;;);
  }

  /* Enable the USB clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
}


#define RCC_APB2Periph_ALLGPIO              (RCC_APB2Periph_GPIOA \
                                               | RCC_APB2Periph_GPIOB \
                                               | RCC_APB2Periph_GPIOC \
                                               | RCC_APB2Periph_GPIOD \
                                               | RCC_APB2Periph_GPIOE )
void GPIO_AINConfig(void)
{

  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ALLGPIO, ENABLE);

  /* Configure all GPIO port pins in Analog Input mode (floating input trigger OFF) */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;

  //GPIO_Init(GPIOA, &GPIO_InitStructure); // Some GPIOA pins are used for JTAG and USB
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  GPIO_Init(GPIOE, &GPIO_InitStructure);

}

void Enter_LowPowerMode(void)
{
  /* Set the device state to suspend */
  bDeviceState = SUSPENDED;
  
  /* Clear EXTI Line18 pending bit */
  EXTI_ClearITPendingBit(EXTI_Line18);
  
  /* Request to enter STOP mode with regulator in low power mode */
  PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
}

void Leave_LowPowerMode(void)
{
  DEVICE_INFO *pInfo = &Device_Info;
  
  /* Set the device state to the correct state */
  if (pInfo->Current_Configuration != 0)
  {
    /* Device configured */
    bDeviceState = CONFIGURED;
  }
  else
  {
    bDeviceState = ATTACHED;
  }

  /*Enable SystemCoreClock*/
  SystemInit();
}


void USB_Interrupts_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* 2 bit for pre-emption priority, 2 bits for subpriority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

  /* Enable the USB interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);


  /* Enable the USB Wake-up interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = USBWakeUp_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_Init(&NVIC_InitStructure);   

}

void USB_Cable_Config (FunctionalState NewState)
{

}

