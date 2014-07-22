#include "usb_lib.h"
#include "usb_conf.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "usb_pwr.h"
#include "hw_config.h"

#include "xhc_dev.h"

static uint32_t ProtocolValue = 0;
static uint32_t IdleValue = 0;

#define SET_REPORT_SIZE         8
uint8_t Report_Buf[SET_REPORT_SIZE];  

DEVICE Device_Table =
  {
    EP_NUM,
    1
  };

DEVICE_PROP Device_Property =
  {
    HID_init,
    HID_Reset,
    HID_Status_In,
    NOP_Process, /* HID Status_Out */
    HID_Data_Setup,
    HID_NoData_Setup,
    HID_Get_Interface_Setting,
    HID_GetDeviceDescriptor,
    HID_GetConfigDescriptor,
    HID_GetStringDescriptor,
    0,
    0x40 /*MAX PACKET SIZE*/
  };

USER_STANDARD_REQUESTS User_Standard_Requests =
  {
    NOP_Process,                /* GetConfiguration */
    HID_SetConfiguration,
    NOP_Process,                /* GetInterface */
    NOP_Process,                /* SetInterface */
    NOP_Process,                /* GetStatus */
    NOP_Process,                /* ClearFeature */
    NOP_Process,                /* SetEndPointFeature */
    NOP_Process,                /* SetDeviceFeature */
    HID_SetDeviceAddress
  };

ONE_DESCRIPTOR Device_Descriptor =
  {
    (uint8_t*)HID_DeviceDescriptor,
    sizeof( HID_DeviceDescriptor )
  };

ONE_DESCRIPTOR Config_Descriptor =
  {
    (uint8_t*)HID_ConfigDescriptor,
    sizeof( HID_ConfigDescriptor )
  };

ONE_DESCRIPTOR Report_Descriptor =
  {
    (uint8_t *)HID_ReportDescriptor,
    sizeof( HID_ReportDescriptor )
  };

ONE_DESCRIPTOR Hid_Descriptor =
  {
    (uint8_t*)HID_ConfigDescriptor + HID_OFF_HID_DESC,
    HID_SIZ_HID_DESC
  };

ONE_DESCRIPTOR String_Descriptor[2] =
  {
    {(uint8_t*)HID_StringLangID, sizeof( HID_StringLangID ) },
    {(uint8_t*)HID_StringVendor, sizeof( HID_StringVendor ) },
  };

void HID_init(void)
{
  pInformation->Current_Configuration = 0;
  /* Connect the device */
  PowerOn();

  /* Perform basic device initialization operations */
  USB_SIL_Init();

  bDeviceState = UNCONNECTED;
}

void HID_Reset(void)
{
  /* Set Joystick_DEVICE as not configured */
  pInformation->Current_Configuration = 0;
  pInformation->Current_Interface = 0;/*the default Interface*/

  /* Current Feature initialization */
  pInformation->Current_Feature = HID_ConfigDescriptor[7];
  SetBTABLE(BTABLE_ADDRESS);
  /* Initialize Endpoint 0 */
  SetEPType(ENDP0, EP_CONTROL);
  SetEPTxStatus(ENDP0, EP_TX_STALL);
  SetEPRxAddr(ENDP0, ENDP0_RXADDR);
  SetEPTxAddr(ENDP0, ENDP0_TXADDR);
  Clear_Status_Out(ENDP0);
  SetEPRxCount(ENDP0, Device_Property.MaxPacketSize);
  SetEPRxValid(ENDP0);

  /* Initialize Endpoint 1 */
  SetEPType(ENDP1, EP_INTERRUPT);
  SetEPTxAddr(ENDP1, ENDP1_TXADDR);
  SetEPTxCount(ENDP1, 0x06 );
  SetEPRxStatus(ENDP1, EP_RX_DIS);
  SetEPTxStatus(ENDP1, EP_TX_NAK);

  /* Set this device to response on default address */
  SetDeviceAddress(0);
  bDeviceState = ATTACHED;
}


void HID_SetConfiguration(void)
{
  if ( Device_Info.Current_Configuration != 0)
  {
    bDeviceState = CONFIGURED;
  }
}

void HID_SetDeviceAddress (void)
{
  bDeviceState = ADDRESSED;
}

/* process input data here */
void HID_Status_In( void )
{
  if( Report_Buf[0] != 6 )
    return;
  /* decode next junk */
  xhc_recv( &Report_Buf[1] );
}

uint8_t *HID_SetReport_Feature(uint16_t Length)
{
  /* ask for report len ? */
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = SET_REPORT_SIZE;
    return NULL;
  }
  else
  {
    return Report_Buf;
  }
}

RESULT HID_Data_Setup(uint8_t RequestNo)
{
  uint8_t *(*CopyRoutine)(uint16_t);

  CopyRoutine = NULL;
  if ((RequestNo == GET_DESCRIPTOR)
      && (Type_Recipient == (STANDARD_REQUEST | INTERFACE_RECIPIENT))
      && (pInformation->USBwIndex0 == 0))
  {
    if (pInformation->USBwValue1 == REPORT_DESCRIPTOR)
    {
      CopyRoutine = HID_GetReportDescriptor;
    }
    else if (pInformation->USBwValue1 == HID_DESCRIPTOR_TYPE)
    {
      CopyRoutine = HID_GetHIDDescriptor;
    }

  } /* End of GET_DESCRIPTOR */

  /*** GET_PROTOCOL, SET_REPORT ***/
  else if ( Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT) )
  {
    switch( RequestNo )
    {
      case GET_PROTOCOL:
        CopyRoutine = HID_GetProtocolValue;
      break;
      case GET_IDLE:
        CopyRoutine = HID_GetIdleValue;
      break;
      case SET_REPORT:
        CopyRoutine = HID_SetReport_Feature;
      break;
    }
  }
  if (CopyRoutine == NULL)
  {
    return USB_UNSUPPORT;
  }
  pInformation->Ctrl_Info.CopyData = CopyRoutine;
  pInformation->Ctrl_Info.Usb_wOffset = 0;
  (*CopyRoutine)(0);
  return USB_SUCCESS;
}

RESULT HID_NoData_Setup(uint8_t RequestNo)
{
  if( Type_Recipient == (CLASS_REQUEST | INTERFACE_RECIPIENT ) )
  {
    switch( RequestNo )
    {
      case SET_PROTOCOL:
        return HID_SetProtocol();
      case SET_IDLE:
        /* dummy save duration */
        IdleValue = pInformation->USBwValue1;
				return USB_SUCCESS;
    }
  }

  return USB_UNSUPPORT;
}

uint8_t *HID_GetDeviceDescriptor(uint16_t Length)
{
  /* PID offset */
  uint16_t *dev_id_ptr = (uint16_t *)&Device_Descriptor.Descriptor[10];
  if( g_hw_type == DEV_WHB04 )
  {
      *dev_id_ptr = WHB04_PID;
  }
  else if( g_hw_type == DEV_WHB03 )
  {
      *dev_id_ptr = WHB03_PID;
  }
  return Standard_GetDescriptorData(Length, &Device_Descriptor);
}

uint8_t *HID_GetConfigDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &Config_Descriptor);
}

uint8_t *HID_GetStringDescriptor(uint16_t Length)
{
  uint8_t wValue0 = pInformation->USBwValue0;
  if (wValue0 > 1)
  {
    return NULL;
  }
  else
  {
    return Standard_GetDescriptorData(Length, &String_Descriptor[wValue0]);
  }
}

uint8_t *HID_GetReportDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &Report_Descriptor);
}

uint8_t *HID_GetHIDDescriptor(uint16_t Length)
{
  return Standard_GetDescriptorData(Length, &Hid_Descriptor);
}

RESULT HID_Get_Interface_Setting(uint8_t Interface, uint8_t AlternateSetting)
{
  if (AlternateSetting > 0)
  {
    return USB_UNSUPPORT;
  }
  else if (Interface > 0)
  {
    return USB_UNSUPPORT;
  }
  return USB_SUCCESS;
}

RESULT HID_SetProtocol(void)
{
  uint8_t wValue0 = pInformation->USBwValue0;
  ProtocolValue = wValue0;
  return USB_SUCCESS;
}

uint8_t *HID_GetProtocolValue(uint16_t Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = 1;
    return NULL;
  }
  else
  {
    return (uint8_t *)(&ProtocolValue);
  }
}

uint8_t *HID_GetIdleValue(uint16_t Length)
{
  if (Length == 0)
  {
    pInformation->Ctrl_Info.Usb_wLength = 1;
    return NULL;
  }
  else
  {
    return (uint8_t *)(&IdleValue);
  }
}
