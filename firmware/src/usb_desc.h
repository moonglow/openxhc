#pragma once

/* include ONLY for usb_prop.h */

#define HID_SIZ_HID_DESC                        0x09
#define HID_OFF_HID_DESC                        0x12
#define HID_SIZ_CONFIG_DESC                     0x22


/* USB Standard Device Descriptor */
uint8_t HID_DeviceDescriptor[] =
{
    0x12, 	/* bLength 		*/
    0x01, 	/* bDescriptorType      */
    0x10,0x01, 	/* bcdUSB 		*/
    0x00, 	/* bDeviceClass 	*/
    0x00, 	/* bDeviceSubClass 	*/
    0x00, 	/* bDeviceProtocol 	*/
    0x40, 	/* bMaxPacketSize0 	*/
    0xCE,0x10, 	/* idVendor 		*/
    0x6E,0xEB, 	/* idProduct 		*/
    0x00,0x00, 	/* bcdDevice 		*/
    0x01, 	/* iManufacturer 	*/
    0x00, 	/* iProduct 		*/
    0x00, 	/* iSerialNumber 	*/
    0x01, 	/* bNumConfigurations   */
};

const uint8_t HID_ConfigDescriptor[] = 
{
  /* CONFIG DESCRIPTOR */
  0x09,	        /* bLength */
  0x02,	        /* bDescriptorType (Configuration)*/
  0x22, 0x00,	/* wTotalLength ( size of all structure ) */
  0x01,	        /* bNumInterfaces */
  0x01,	        /* bConfigurationValue */
  0x00,	        /* iConfiguration */
  0x80,	        /* bmAttributes   (Bus-powered Device) */
  0x32,	        /* bMaxPower   (100 mA) */
  
  /* INTERFACE DESCRIPTOR */
  0x09,	        /* bLength */
  0x04,	        /* bDescriptorType (Interface)*/
  0x00,	        /* bInterfaceNumber */
  0x00,	        /* bAlternateSetting */
  0x01,	        /* bNumEndPoints */
  0x03,	        /* bInterfaceClass (HID) */
  0x00,	        /* bInterfaceSubClass */
  0x00,	        /* bInterfaceProtocol */   
  0x00,	        /* iInterface */
  
  /* HID Descriptor ( offset 0x12 )*/
  0x09,         /* bLength */
  0x21,         /* bDescriptorType (HID) */
  0x10,0x01,    /* bcdHID (1.10) */
  0x00,         /* bCountryCode */
  0x01,         /* bNumDescriptors */
  0x22,         /* bDescriptorType ( Report ) */
  0x2E,0x00,    /* wDescriptorLength ( Report Size ) */
  
  /* ENDPOINT DESCRIPTOR */
  0x07,	        /* bLength */
  0x05,	        /* bDescriptorType ( Endpoint )*/
  0x81,	        /* bEndpointAddress (IN Endpoint 1) */
  0x03,	        /* bmAttributes	( Interrupt ) */
  0x40, 0x00,	/* wMaxPacketSize   (64 Bytes) */
  0x02,	        /* bInterval ( 2 ms )*/
};

const uint8_t HID_ReportDescriptor[] =
{
    0x06,0x00,0xFF, 	        /* Usage Page (Vendor-Defined 1) */						
    0x09,0x01, 			/* Usage (Vendor-Defined 1) */								
    0xA1,0x01, 			/* Collection (Application) */ 									
    0x85,0x04, 				/* Report ID (4) */								
    0x09,0x01, 				/* Usage (Vendor-Defined 1) */ 								
    0x15,0x00, 				/* Logical Minimum (0) */ 									
    0x26,0xFF,0x00, 		        /* Logical Maximum (255) */ 									
    0x95,0x05, 				/* Report Count (5) */ 										
    0x75,0x08, 				/* Report Size (8) */ 										
    0x81,0x02, 				/* Input (Data,Var,Abs,NWrp,Lin,Pref,NNul,Bit) */				
    0xC0, 			/* End Collection */ 	
    
    0x06,0x00,0xFF, 	        /* Usage Page (Vendor-Defined 1) */ 								
    0x09,0x01, 			/* Usage (Vendor-Defined 1) */ 									
    0xA1,0x01, 			/* Collection (Application) */ 									
    0x85,0x06, 				/* Report ID (6) */ 											
    0x09,0x01, 				/* Usage (Vendor-Defined 1) */ 								
    0x15,0x00, 				/* Logical Minimum (0) */ 									
    0x26,0xFF,0x00, 		        /* Logical Maximum (255) */ 									
    0x95,0x07, 				/* Report Count (7) */ 										
    0x75,0x08, 				/* Report Size (8) */ 										
    0xB1,0x06, 				/* Feature (Data,Var,Rel,NWrp,Lin,Pref,NNul,NVol,Bit) */ 		
    0xC0, 			/* End Collection */ 
};

/* USB String Descriptors */
const uint8_t HID_StringLangID[] =
{
    0x04,       /* bLength */
    0x03,       /* bDescriptorType (String)*/
    0x09, 0x04  /* 0x0409 US */
};

#define HID_SIZ_STRING_VENDOR              20
const uint8_t HID_StringVendor[] =
{
    HID_SIZ_STRING_VENDOR, /* bLength */
    0x03,                 /* bDescriptorType (String)*/
    /* string len - 18 bytes */
    'K', 0, 'T', 0, 'U', 0, 'R', 0, 'T', 0, '.', 0, 'L', 0, 'T', 0, 'D', 0
};
