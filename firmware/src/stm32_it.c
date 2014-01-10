
#include <stm32f10x.h>

#include "usb_istr.h"
#include "usb_lib.h"
#include "usb_pwr.h"



void HardFault_Handler(void)
{
  for( ;; );
}

void MemManage_Handler(void)
{
  for( ;; );
}

void BusFault_Handler(void)
{
 for( ;; );
}

void UsageFault_Handler(void)
{
 for( ;; );
}

void EXTI9_5_IRQHandler(void)
{
#if 0
  if (EXTI_GetITStatus(KEY_BUTTON_EXTI_LINE) != RESET)
  {
    /* Check if the remote wakeup feature is enabled (it could be disabled 
        by the host through ClearFeature request) */
    if (pInformation->Current_Feature & 0x20) 
    {      
      pInformation->Current_Feature &= ~0x20;  
      /* Exit low power mode and re-configure clocks */
      Resume(RESUME_INTERNAL);
    }
  
    /* Clear the EXTI line pending bit */
    EXTI_ClearITPendingBit(KEY_BUTTON_EXTI_LINE);
  }
#endif
}

void USB_LP_CAN1_RX0_IRQHandler(void)
{
  USB_Istr();
}

void USBWakeUp_IRQHandler(void)
{
  EXTI_ClearITPendingBit(EXTI_Line18);
}

