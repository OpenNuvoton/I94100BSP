/******************************************************************************
 * @file     descriptors.c
 * @brief   USBD driver source file
 * @version  1.0.0
 * @date     25, Nov, 2019
 *
 * @note
 * Copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
/*!<Includes */
#include <string.h>
#include "Platform.h"
#include "hid_request.h"
 
#define WBVAL(x) (x&0xFF), ((x >>8) & 0xFF) 
#define B3VAL(x) (x&0xFF), ((x >>8) & 0xFF), ((x >>16) & 0xFF)  


uint8_t ReportDescriptor[] = 
{
	0x06, 0x00, 0xFF,  			// Usage Page (Vendor Defined 0xFF00)
	0x09, 0x01,        			// Usage (0x01)
	0xA1, 0x01,        			// Collection (Application)
	
	0x85, IF0_INPUT_RPTID,   	//   Report ID (2)
	0x09, 0x01,        			//   Usage (0x01)
	0x15, 0x00,        			//   Logical Minimum (0)
	0x26, 0xFF, 0x00,  			//   Logical Maximum (255)
	0x75, 0x08,        			//   Report Size (8)
	0x95, 0x3F,        			//   Report Count (63)
	0x81, 0x02,        			//   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0xC0,              	// End Collection
};
	

#define  ReportDesc_Size  sizeof(ReportDescriptor)


/*----------------------------------------------------------------------------*/
/*!<USB Device Descriptor */
uint8_t gu8DeviceDescriptor[] = 
{
    LEN_DEVICE,     			/* bLength */
    DESC_DEVICE,    			/* bDescriptorType */
    0x10, 0x01,     			/* bcdUSB */
    0x00,           			/* bDeviceClass */
    0x00,           			/* bDeviceSubClass */
    0x00,           			/* bDeviceProtocol */
    EP0_MAX_PKT_SIZE,   		/* bMaxPacketSize0 */
	WBVAL(USBD_VID),      		/* idVendor */
	WBVAL(USBD_PID),      		/* idProduct */
    0x00, 0x00,     			/* bcdDevice */
    0x01,           			/* iManufacture */
    0x02,           			/* iProduct */
    0x00,           			/* iSerialNumber - no serial */
    0x01            			/* bNumConfigurations */
};


/*!<USB Configure Descriptor */
uint8_t gu8ConfigDescriptor[] = 
{
    LEN_CONFIG,     								/* bLength */
    DESC_CONFIG,    								/* bDescriptorType */
    
	/* wTotalLength */
	WBVAL(LEN_CONFIG_AND_SUBORDINATE),
	USBD_NUM_INTERFACE, 					  		/* bNumInterfaces */
    0x01,           								/* bConfigurationValue */
    0x00,           								/* iConfiguration */
    0x80 | (USBD_SELF_POWERED << 6) | (USBD_REMOTE_WAKEUP << 5),/* bmAttributes */
    USBD_MAX_POWER, 								/* MaxPower */
	
	/* IF_0 */ 
    LEN_INTERFACE,  								/* bLength */
    DESC_INTERFACE, 								/* bDescriptorType */
    0x00,           								/* bInterfaceNumber = 0 */
    0x00,           								/* bAlternateSetting */
    0x01,           								/* bNumEndpoints = 1 */
    0x03,           								/* bInterfaceClass */
    0x00,           								/* bInterfaceSubClass */
    HID_NONE,       								/* bInterfaceProtocol */
    0x00,           								/* iInterface */

    /* HID Descriptor */
    LEN_HID,        								/* Size of this descriptor in UINT8. */
    DESC_HID,       								/* HID descriptor type. */
	WBVAL(0x0110),            						/* HID Class Spec. release number. */
    0x00,           								/* H/W target country. */
    0x01,           								/* Number of HID class descriptors to follow. */
    DESC_HID_RPT,   								/* Descriptor type. */
	WBVAL(ReportDesc_Size),     				/* Total length of report descriptor. */

    /* EP Descriptor */
    LEN_ENDPOINT,   								/* bLength */
    DESC_ENDPOINT,  								/* bDescriptorType */
	EP2_ADDR,	
    EP_INT,         								/* bmAttributes */
	WBVAL(EP2_MAX_PKT_SIZE),
	HID_DEFAULT_INT_IN_INTERVAL,    			/* bInterval */		
};


/*!<USB Language String Descriptor */
uint8_t gu8StringLang[4] = 
{
    4,              /* bLength */
    DESC_STRING,    /* bDescriptorType */
    0x09, 0x04
};

/*!<USB Vendor String Descriptor */
uint8_t gu8VendorStringDesc[] = 
{
    16,
    DESC_STRING,
    'N', 0, 
	'u', 0, 
	'v', 0, 
	'o', 0, 
	't', 0, 
	'o', 0, 
	'n', 0
};

/*!<USB Product String Descriptor */
uint8_t gu8ProductStringDesc[] = 
{
    18,
    DESC_STRING,
    'H', 0, 
	'I', 0, 
	'D', 0, 
	' ', 0, 
	'D', 0, 
	'e', 0, 
	'm', 0,
	'o', 0
};

/*!<USB BOS Descriptor */
uint8_t gu8BOSDescriptor[] = 
{
    LEN_BOS,        /* bLength */
    DESC_BOS,       /* bDescriptorType */
    /* wTotalLength */
    0x0C & 0x00FF,
    (0x0C & 0xFF00) >> 8,
    0x01,           /* bNumDeviceCaps */

    /* Device Capability */
    0x7,            /* bLength */
    DESC_CAPABILITY,/* bDescriptorType */
    CAP_USB20_EXT,  /* bDevCapabilityType */
    0x02, 0x00, 0x00, 0x00  /* bmAttributes */
};

uint8_t *gpu8UsbString[4] = 
{
    gu8StringLang,
    gu8VendorStringDesc,
    gu8ProductStringDesc,
    NULL,
};

uint8_t *gu8UsbHidReport[3] = 
{
		ReportDescriptor,
    NULL,
    NULL,
};

uint32_t gu32UsbHidReportLen[3] = 
{
	sizeof(ReportDescriptor),		
    0,
    0,
};

uint32_t gu32ConfigHidDescIdx[3] = 
{
    (LEN_CONFIG+LEN_INTERFACE),
    0,
    0,
};


const S_USBD_INFO_T gsInfo = 
{
    (uint8_t *)gu8DeviceDescriptor,
    (uint8_t *)gu8ConfigDescriptor,
    (const uint8_t **)gpu8UsbString,
    (const uint8_t **)gu8UsbHidReport,
    (uint8_t *)gu8BOSDescriptor,
    (uint32_t *)gu32UsbHidReportLen,
    (uint32_t *)gu32ConfigHidDescIdx
};

