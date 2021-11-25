/******************************************************************************
 * @file     msc_desc.c
 * @brief    Mass storage clasee description.
 * @version  1.0.0
 * @date     25, Nov, 2019
 *
 * @note
 * Copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
 
/*!<Includes */
#include <string.h>
#include "Platform.h"
#include "massstorage.h"
#include "usbd_bot.h"

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
    EP0_MAX_PKT_SIZE,   	/* bMaxPacketSize0 */ 
		LOW_BYTE(USBD_VID),		/* idVendor */
	  HIGH_BYTE(USBD_VID),  
		LOW_BYTE(USBD_PID),		/* idProduct */
		HIGH_BYTE(USBD_PID),	
    0x00, 0x00,     			/* bcdDevice */
    0x01,           			/* iManufacture */
    0x02,           			/* iProduct */
    0x03,           			/* iSerialNumber - serial string */
    0x01            			/* bNumConfigurations */
};


/*!<USB Configure Descriptor */
const uint8_t gu8ConfigDescriptor[] = {
    LEN_CONFIG,                                         // bLength
    DESC_CONFIG,                                        // bDescriptorType
    (LEN_CONFIG + LEN_INTERFACE + LEN_ENDPOINT * 2), 0x00, // wTotalLength
    0x01,                                               // bNumInterfaces
    0x01,                                               // bConfigurationValue
    0x00,                                               // iConfiguration
    0xC0,                                               // bmAttributes
//	  0x40,                                               // bmAttributes 
    0x32,                                               // MaxPower

    /* const BYTE cbyInterfaceDescriptor[LEN_INTERFACE] = */
    LEN_INTERFACE,  					// bLength
    DESC_INTERFACE, 					// bDescriptorType
    0x00,     								// bInterfaceNumber
    0x00,     								// bAlternateSetting
    0x02,     								// bNumEndpoints
    USB_DEVICE_CLASS_STORAGE, // bInterfaceClass
    MSC_SUBCLASS_SCSI,     		// bInterfaceSubClass
    MSC_PROTOCOL_BULK_ONLY,   // bInterfaceProtocol
    0x00,     								// iInterface

    /* Descriptor = */
    LEN_ENDPOINT,           	// bLength
    DESC_ENDPOINT,          	// bDescriptorType
    (BULK_IN_EP | EP_INPUT), 	// bEndpointAddress
    EP_BULK,                	// bmAttributes
    EP2_MAX_PKT_SIZE, 0x00,  	// wMaxPacketSize
    0x00,                   	// bInterval

    /* Descriptor */
    LEN_ENDPOINT,           	// bLength
    DESC_ENDPOINT,          	// bDescriptorType
    BULK_OUT_EP,              // bEndpointAddress
    EP_BULK,                	// bmAttributes
    EP3_MAX_PKT_SIZE, 0x00,  	// wMaxPacketSize
    0x00                    	// bInterval
};


/*!<USB Language String Descriptor */
const uint8_t gu8StringLang[4] = {
    4,              /* bLength */
    DESC_STRING,    /* bDescriptorType */
    0x09, 0x04
};

/*!<USB Vendor String Descriptor */
const uint8_t gu8VendorStringDesc[16] = {
    16,
    DESC_STRING,
    'N', 0, 'u', 0, 'v', 0, 'o', 0, 't', 0, 'o', 0, 'n', 0
};

/*!<USB Product String Descriptor */
const uint8_t gu8ProductStringDesc[22] = {
    22,             /* bLength          */
    DESC_STRING,    /* bDescriptorType  */
    'U', 0, 'S', 0, 'B', 0, ' ', 0, 'D', 0, 'e', 0, 'v', 0, 'i', 0, 'c', 0, 'e', 0
};


uint8_t gu8StringSerial[26] = {
    26,             // bLength
    DESC_STRING,    // bDescriptorType
    'A', 0, 
	  '0', 0, 
	  '0', 0, 
	  '0', 0, 
	  '0', 0,   
	  '8', 0, 
	  '0', 0, 
	  '4', 0, 
	  '0', 0, 
	  '1', 0, 
	  '1', 0, 
	  '5', 0

};


const uint8_t *gpu8UsbString[4] = {
    gu8StringLang,
    gu8VendorStringDesc,
    gu8ProductStringDesc,
    gu8StringSerial
};

const S_USBD_INFO_T gsInfo = {
    (uint8_t *)gu8DeviceDescriptor,
    (uint8_t *)gu8ConfigDescriptor,
    (const uint8_t **)gpu8UsbString,
    NULL,
    NULL,
    NULL,
    NULL
};

