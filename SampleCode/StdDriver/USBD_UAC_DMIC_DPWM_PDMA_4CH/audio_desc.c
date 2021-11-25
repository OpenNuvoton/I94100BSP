/******************************************************************************
 * @file     audio_desc.c
 * @brief    M480 series USBD driver source file
 * @version  1.0.0
 * @date     August, 2018
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
/*!<Includes */
#include <string.h>
#include "Platform.h"
#include "usbd_audio.h"
#include "audioclass.h"


#define WBVAL(x) (x&0xFF), ((x >>8) & 0xFF) 
#define B3VAL(x) (x&0xFF), ((x >>8) & 0xFF), ((x >>16) & 0xFF)  

const uint8_t gu8HidReportDesc[67] = 
{
    0x05, 0x0C,      // Usage Page (Consumer)
    0x09, 0x01,      // Usage(Consumer Control)
    0xA1, 0x01,      // Collection(Application )
    0x15, 0x00,      // Logical Minimum(0x0 )
    0x25, 0x01,      // Logical Maximum(0x1 )
    0x09, 0xE2,      // Usage(Mute)
    0x09, 0xE9,      // Usage(Volume Increment)
    0x09, 0xEA,      // Usage(Volume Decrement)
    0x75, 0x01,      // Report Size(0x1 )
    0x95, 0x03,      // Report Count(0x3 )
    0x81, 0x02,      // Input(Data, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Bit Field)
    0x75, 0x01,      // Report Size(0x1 )
    0x95, 0x05,      // Report Count(0x5 )
    0x81, 0x03,      // Input(Constant, Variable, Absolute) - Padding 
	
    0x09, 0xB0,      // Usage(Play)
    0x09, 0xB7,      // Usage(Stop)
    0x09, 0xCD,      // Usage(Play/Pause)
    0x09, 0xB5,      // Usage(Scan Next Track)
    0x09, 0xB6,      // Usage(Scan Previous Track)
    0x09, 0xB2,      // Usage(Record)
    0x09, 0xB4,      // Usage(Rewind)
    0x09, 0xB3,      // Usage(Fast Forward)
    0x75, 0x01,      // Report Size(0x1 )
    0x95, 0x08,      // Report Count(0x8 )
    0x81, 0x02,      // Input(Data, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Bit Field)
		
    0x09, 0x00,      // Usage(Undefined)
    0x75, 0x08,      // Report Size(0x8 )
    0x95, 0x06,      // Report Count(0x6 )
    0x81, 0x02,      // Input(Data, Variable, Absolute, No Wrap, Linear, Preferred State, No Null Position, Bit Field)
		
    0x09, 0x00,      // Usage(Undefined)
    0x75, 0x08,      // Report Size(0x8 )
    0x95, 0x08,      // Report Count(0x8 )
    0x91, 0x00,
    0xC0
};


/*----------------------------------------------------------------------------*/
/*!<USB Device Descriptor */
const uint8_t gu8DeviceDescriptor[] = 
{
		LEN_DEVICE,        /* bLength */
		DESC_DEVICE,       /* bDescriptorType */
		WBVAL(0x0200),     /* bcdUSB */
		0x00,              /* bDeviceClass */
		0x00,              /* bDeviceSubClass */
		0x00,              /* bDeviceProtocol */
		EP0_MAX_PKT_SIZE,  /* bMaxPacketSize0 */
		WBVAL(USBD_VID),   /* idVendor */
		WBVAL(USBD_PID),   /* idProduct */ 
		0x00, 0x00,        /* bcdDevice */
		0x01,              /* iManufacture */
		0x02,              /* iProduct */
		0x03,              /* iSerialNumber
													NOTE: The serial number must be different between each MassStorage device. */
		0x01               /* bNumConfigurations */
};

#define HID_REPORT_DESCRIPTOR_SIZE  sizeof(gu8HidReportDesc) 

/*!<USB Configure Descriptor */
const uint8_t gu8ConfigDescriptor[] = 
{
    LEN_CONFIG,        						// bLength
    DESC_CONFIG,        					// bDescriptorType (Configuration)
    WBVAL(457),  									// wTotalLength 
    0x06,        									// bNumInterfaces 6 
    0x01,        									// bConfigurationValue
    0x00,        									// iConfiguration (String Index)
    USB_CONFIG_BUS_POWERED,       // bmAttributes
    USB_CONFIG_POWER_MA(100),			// bMaxPower 100mA

		/* Standard AC inteface */
    LEN_INTERFACE,        // bLength
    DESC_INTERFACE,        // bDescriptorType (Interface)
    0x00,        // bInterfaceNumber 0
    0x00,        // bAlternateSetting
    0x00,        // bNumEndpoints 0
    USB_DEVICE_CLASS_AUDIO,        // bInterfaceClass (Audio)
    AUDIO_SUBCLASS_AUDIOCONTROL,        // bInterfaceSubClass (Audio Control)
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

    /* Class-spec AC interface descriptor */
    0x0D,           // bLength = 13 
    0x24,           // bDescriptorType (See Next Line)
    0x01,           // bDescriptorSubtype (CS_INTERFACE -> HEADER)
    0x00, 0x01,     // bcdADC 1.00
    0xAA, 0x00,     // wTotalLength
    0x05,           // binCollection 0x05
    0x01,           // baInterfaceNr 1
    0x02,           // baInterfaceNr 2
    0x03,           // baInterfaceNr 3
    0x04,           // baInterfaceNr 4
    0x05,           // baInterfaceNr 5

		/* Input for usb streaming */
    0x0C,           // bLength 
    0x24,           // bDescriptorType:CS_INTERFACE 
    0x02,           // bDescriptorSubType:INPUT_TERMINAL 
    0x01,         	// bTerminalID 
    0x01, 0x01,     // wTerminalType (USB Streaming)
    0x00,           // bAssocTerminal 
    PLAY_CHANNELS,      	    // bNrChannels 
    PLAY_CH_CFG, 0x00,  	// wChannelConfig 
    0x00,           // iChannelNames 
    0x00,           // iTerminal 

		/* Feature Unit */
    0x0A,           // bLength 
    0x24,           // bDescriptorType 
    0x06,           // bDescriptorSubType 
    PLAY_FEATURE_UNITID,           // bUnitID 
    0x01,           // bSourceID 
    0x01,           // bControlSize 
    0x01,           // bmaControls(0) 
    0x00,           // bmaControls(1) 
    0x00,           // bmaControls(2) 
    0x00,           // iFeature */
		
		/* Output for speaker */
    0x09,           // bLength
    0x24,           // bDescriptorType:CS_INTERFACE
    0x03,           // bDescriptorSubType:OUTPUT_TERMINAL
    0x03,           // bTerminalID
    0x01, 0x03,     // wTerminalType: speaker
    0x00,           // bAssocTerminal
    PLAY_FEATURE_UNITID,           // bSourceID
    0x00,           // iTerminal

		/* Input Terminal for microphone No.1 */
    0x0C,        // bLength
    0x24,        // bDescriptorType (See Next Line)
    0x02,        // bDescriptorSubtype (CS_INTERFACE -> INPUT_TERMINAL)
    REC1CH_INPUT_ID,        // bTerminalID
    0x01, 0x02,  // wTerminalType (Microphone)
    0x00,        // bAssocTerminal
    0x01,        // bNrChannels 1
    0x01, 0x00,  // wChannelConfig (Left Front)
    0x00,        // iChannelNames
    0x00,        // iTerminal

		/* Feature Unit */
    0x09,        // bLength
    0x24,        // bDescriptorType (See Next Line)
    0x06,        // bDescriptorSubtype (CS_INTERFACE -> FEATURE_UNIT)
    REC1CH_FEATURE_UNITID,        // bUnitID
    REC1CH_INPUT_ID,        // bSourceID
    0x01,        // bControlSize 1
    0x03, 0x00,  // bmaControls[0] (Mute,Volume)
    0x00,        // iFeature

		/* Output Terminal for usb streaming */
    0x09,        // bLength
    0x24,        // bDescriptorType (See Next Line)
    0x03,        // bDescriptorSubtype (CS_INTERFACE -> OUTPUT_TERMINAL)
    REC1CH_OUTPUT_ID,        // bTerminalID
    0x01, 0x01,  // wTerminalType (USB Streaming)
    0x00,        // bAssocTerminal
    REC1CH_FEATURE_UNITID,        // bSourceID
    0x00,        // iTerminal

		/* Input Terminal for microphone No.2 */
    0x0C,        // bLength
    0x24,        // bDescriptorType (See Next Line)
    0x02,        // bDescriptorSubtype (CS_INTERFACE -> INPUT_TERMINAL)
    REC2CH_INPUT_ID,        // bTerminalID
    0x01, 0x02,  // wTerminalType (Microphone)
    0x00,        // bAssocTerminal
    0x02,        // bNrChannels 2
    0x03, 0x00,  // wChannelConfig (Left and Right Front)
    0x00,        // iChannelNames
    0x00,        // iTerminal

		/* Feature Unit */
    0x0A,        // bLength
    0x24,        // bDescriptorType (See Next Line)
    0x06,        // bDescriptorSubtype (CS_INTERFACE -> FEATURE_UNIT)
    REC2CH_FEATURE_UNITID,        // bUnitID
    REC2CH_INPUT_ID,        // bSourceID
    0x01,        // bControlSize 1
    0x01, 0x02,  // bmaControls[0] (Mute)
    0x02, 0x00,  // bmaControls[1] (Volume)

		/* Output Terminal for usb streaming */
    0x09,        // bLength
    0x24,        // bDescriptorType (See Next Line)
    0x03,        // bDescriptorSubtype (CS_INTERFACE -> OUTPUT_TERMINAL)
    REC2CH_OUTPUT_ID,        // bTerminalID
    0x01, 0x01,  // wTerminalType (USB Streaming)
    0x00,        // bAssocTerminal
    REC2CH_FEATURE_UNITID,        // bSourceID
    0x00,        // iTerminal

		/* Input Terminal for microphone No.3 */
    0x0C,        // bLength
    0x24,        // bDescriptorType (See Next Line)
    0x02,        // bDescriptorSubtype (CS_INTERFACE -> INPUT_TERMINAL)
    REC3CH_INPUT_ID,        // bTerminalID
    0x01, 0x02,  // wTerminalType (Microphone)
    0x00,        // bAssocTerminal
    0x03,        // bNrChannels 3
    0x07, 0x00,  // wChannelConfig (Left and Right Front,Center Front)
    0x00,        // iChannelNames
    0x00,        // iTerminal

		/* Feature Unit */
    0x0B,        // bLength
    0x24,        // bDescriptorType (See Next Line)
    0x06,        // bDescriptorSubtype (CS_INTERFACE -> FEATURE_UNIT)
    REC3CH_FEATURE_UNITID,        // bUnitID
    REC3CH_INPUT_ID,        // bSourceID
    0x01,        // bControlSize 1
    0x01, 0x02,  // bmaControls[0] (Mute)
    0x02, 0x02,  // bmaControls[1] (Volume)
    0x00,        // iFeature

		/* Output Terminal for usb streaming */
    0x09,        // bLength
    0x24,        // bDescriptorType (See Next Line)
    0x03,        // bDescriptorSubtype (CS_INTERFACE -> OUTPUT_TERMINAL)
    REC3CH_OUTPUT_ID,        // bTerminalID
    0x01, 0x01,  // wTerminalType (USB Streaming)
    0x00,        // bAssocTerminal
    REC3CH_FEATURE_UNITID,        // bSourceID
    0x00,        // iTerminal

		/* Input Terminal for microphone No.3 */
    0x0C,        // bLength
    0x24,        // bDescriptorType (See Next Line)
    0x02,        // bDescriptorSubtype (CS_INTERFACE -> INPUT_TERMINAL)
    REC4CH_INPUT_ID,        // bTerminalID
    0x01, 0x02,  // wTerminalType (Microphone)
    0x00,        // bAssocTerminal
    0x04,        // bNrChannels 4
    0x0F, 0x00,  // wChannelConfig (Left and Right Front,Center Front,Low Freq Enh)
    0x00,        // iChannelNames
    0x00,        // iTerminal

		/* Feature Unit */
    0x0C,        // bLength
    0x24,        // bDescriptorType (See Next Line)
    0x06,        // bDescriptorSubtype (CS_INTERFACE -> FEATURE_UNIT)
    REC4CH_FEATURE_UNITID,        // bUnitID
    REC4CH_INPUT_ID,        // bSourceID
    0x01,        // bControlSize 1
    0x01, 0x02,  // bmaControls[0] (Mute)
    0x02, 0x02,  // bmaControls[1] (Volume)
    0x02, 0x00,  // bmaControls[2] (Volume)

		/* Output Terminal for usb streaming */
    0x09,        // bLength
    0x24,        // bDescriptorType (See Next Line)
    0x03,        // bDescriptorSubtype (CS_INTERFACE -> OUTPUT_TERMINAL)
    REC4CH_OUTPUT_ID,        // bTerminalID
    0x01, 0x01,  // wTerminalType (USB Streaming)
    0x00,        // bAssocTerminal
    REC4CH_FEATURE_UNITID,        // bSourceID
    0x00,        // iTerminal

		//-------------------------------------------------------
    /* Standard AS interface 1, alternate 0 */
    LEN_INTERFACE,        // bLength
    DESC_INTERFACE,        // bDescriptorType (Interface)
    0x01,        // bInterfaceNumber 1
    0x00,        // bAlternateSetting
    0x00,        // bNumEndpoints 0
    USB_DEVICE_CLASS_AUDIO,        // bInterfaceClass (Audio)
    AUDIO_SUBCLASS_AUDIOSTREAMING,        // bInterfaceSubClass (Audio Streaming)
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

		/* Standard AS interface 1, alternate 1 */
    LEN_INTERFACE,        // bLength
    DESC_INTERFACE,        // bDescriptorType (Interface)
    0x01,        // bInterfaceNumber 1
    0x01,        // bAlternateSetting
    0x01,        // bNumEndpoints 1
    USB_DEVICE_CLASS_AUDIO,        // bInterfaceClass (Audio)
    AUDIO_SUBCLASS_AUDIOSTREAMING,        // bInterfaceSubClass (Audio Streaming)
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

		/* Class-spec AS interface */
    0x07,        // bLength
    0x24,        // bDescriptorType (See Next Line)
    0x01,        // bDescriptorSubtype (CS_INTERFACE -> AS_GENERAL)
    0x12,        // bTerminalLink
    0x01,        // bDelay 1
    0x01, 0x00,  // wFormatTag (PCM)

		/* Type I format type Descriptor */
    0x0B,        // bLength
    0x24,        // bDescriptorType (See Next Line)
    0x02,        // bDescriptorSubtype (CS_INTERFACE -> FORMAT_TYPE)
    0x01,        // bFormatType 1
    0x01,        // bNrChannels (Mono)
    0x03,        // bSubFrameSize 3
    0x18,        // bBitResolution 24
    0x01,        // bSamFreqType 1
    B3VAL(REC_RATE),  // tSamFreq[1] 16000 Hz

		/* Standard AS ISO Audio Data Endpoint */
    0x09,        // bLength
    0x05,        // bDescriptorType (See Next Line)
    EP2 | EP_INPUT,        // bEndpointAddress (IN/D2H)
    0x0D,        // bmAttributes (Isochronous, Sync, Data EP)
    WBVAL(EP2_MAX_PKT_SIZE),  // wMaxPacketSize 64 
    0x01,        // bInterval 1 (unit depends on device speed)
    0x00,        // bRefresh
    0x00,        // bSyncAddress

		/* Class-spec AS ISO Audio Data endpoint Descriptor */
    0x07,        // bLength
    0x25,        // bDescriptorType (See Next Line)
    0x01,        // bDescriptorSubtype (CS_ENDPOINT -> EP_GENERAL)
    0x00,        // bmAttributes (None)
    0x00,        // bLockDelayUnits
    0x00, 0x00,  // wLockDelay 0

		/* Standard AS interface 2, alternate 0 */
    LEN_INTERFACE,        // bLength
    DESC_INTERFACE,        // bDescriptorType (Interface)
    0x02,        // bInterfaceNumber 2
    0x00,        // bAlternateSetting
    0x00,        // bNumEndpoints 0
    USB_DEVICE_CLASS_AUDIO,        // bInterfaceClass (Audio)
    AUDIO_SUBCLASS_AUDIOSTREAMING,        // bInterfaceSubClass (Audio Streaming)
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

		/* Standard AS interface 2, alternate 1 */
    LEN_INTERFACE,        // bLength
    DESC_INTERFACE,        // bDescriptorType (Interface)
    0x02,        // bInterfaceNumber 2
    0x01,        // bAlternateSetting
    0x01,        // bNumEndpoints 1
    USB_DEVICE_CLASS_AUDIO,        // bInterfaceClass (Audio)
    AUDIO_SUBCLASS_AUDIOSTREAMING,        // bInterfaceSubClass (Audio Streaming)
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

		/* Class-spec AS interface */
    0x07,        // bLength
    0x24,        // bDescriptorType (See Next Line)
    0x01,        // bDescriptorSubtype (CS_INTERFACE -> AS_GENERAL)
    0x22,        // bTerminalLink
    0x01,        // bDelay 1
    0x01, 0x00,  // wFormatTag (PCM)

		/* Type I format type Descriptor */
    0x0B,        // bLength
    0x24,        // bDescriptorType (See Next Line)
    0x02,        // bDescriptorSubtype (CS_INTERFACE -> FORMAT_TYPE)
    0x01,        // bFormatType 1
    0x02,        // bNrChannels (Stereo)
    0x03,        // bSubFrameSize 3
    0x18,        // bBitResolution 24
    0x01,        // bSamFreqType 1
    B3VAL(REC_RATE),  // tSamFreq[1] 16000 Hz

		/* Standard AS ISO Audio Data Endpoint */
    0x09,        // bLength
    0x05,        // bDescriptorType (See Next Line)
    EP6 | EP_INPUT,        // bEndpointAddress (IN/D2H)
    0x0D,        // bmAttributes (Isochronous, Sync, Data EP)
    WBVAL(EP6_MAX_PKT_SIZE),  // wMaxPacketSize 128
    0x01,        // bInterval 1 (unit depends on device speed)
    0x00,        // bRefresh
    0x00,        // bSyncAddress

		/* Class-spec AS ISO Audio Data endpoint Descriptor */
    0x07,        // bLength
    0x25,        // bDescriptorType (See Next Line)
    0x01,        // bDescriptorSubtype (CS_ENDPOINT -> EP_GENERAL)
    0x00,        // bmAttributes (None)
    0x00,        // bLockDelayUnits
    0x00, 0x00,  // wLockDelay 0

		/* Standard AS interface 3, alternate 0 */
    LEN_INTERFACE,        // bLength
    DESC_INTERFACE,        // bDescriptorType (Interface)
    0x03,        // bInterfaceNumber 3
    0x00,        // bAlternateSetting
    0x00,        // bNumEndpoints 0
    USB_DEVICE_CLASS_AUDIO,        // bInterfaceClass (Audio)
    AUDIO_SUBCLASS_AUDIOSTREAMING,        // bInterfaceSubClass (Audio Streaming)
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

		/* Standard AS interface 3, alternate 1 */
    LEN_INTERFACE,        // bLength
    DESC_INTERFACE,        // bDescriptorType (Interface)
    0x03,        // bInterfaceNumber 3
    0x01,        // bAlternateSetting
    0x01,        // bNumEndpoints 1
    USB_DEVICE_CLASS_AUDIO,        // bInterfaceClass (Audio)
    AUDIO_SUBCLASS_AUDIOSTREAMING,        // bInterfaceSubClass (Audio Streaming)
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

    0x07,        // bLength
    0x24,        // bDescriptorType (See Next Line)
    0x01,        // bDescriptorSubtype (CS_INTERFACE -> AS_GENERAL)
    0x32,        // bTerminalLink
    0x01,        // bDelay 1
    0x01, 0x00,  // wFormatTag (PCM)

		/* Type I format type Descriptor */
    0x0B,        // bLength
    0x24,        // bDescriptorType (See Next Line)
    0x02,        // bDescriptorSubtype (CS_INTERFACE -> FORMAT_TYPE)
    0x01,        // bFormatType 1
    0x03,        // bNrChannels 3
    0x03,        // bSubFrameSize 3
    0x18,        // bBitResolution 24
    0x01,        // bSamFreqType 1
    B3VAL(REC_RATE),  // tSamFreq[1] 16000 Hz

		/* Standard AS ISO Audio Data Endpoint */
    0x09,        // bLength
    0x05,        // bDescriptorType (See Next Line)
    EP7 | EP_INPUT,        // bEndpointAddress (IN/D2H)
    0x0D,        // bmAttributes (Isochronous, Sync, Data EP)
    WBVAL(EP7_MAX_PKT_SIZE),  // wMaxPacketSize 180
    0x01,        // bInterval 1 (unit depends on device speed)
    0x00,        // bRefresh
    0x00,        // bSyncAddress

		/* Class-spec AS ISO Audio Data endpoint Descriptor */
    0x07,        // bLength
    0x25,        // bDescriptorType (See Next Line)
    0x01,        // bDescriptorSubtype (CS_ENDPOINT -> EP_GENERAL)
    0x00,        // bmAttributes (None)
    0x00,        // bLockDelayUnits
    0x00, 0x00,  // wLockDelay 0

		/* Standard AS interface 4, alternate 0 */
    LEN_INTERFACE,        // bLength
    DESC_INTERFACE,        // bDescriptorType (Interface)
    0x04,        // bInterfaceNumber 4
    0x00,        // bAlternateSetting
    0x00,        // bNumEndpoints 0
    USB_DEVICE_CLASS_AUDIO,        // bInterfaceClass (Audio)
    AUDIO_SUBCLASS_AUDIOSTREAMING,        // bInterfaceSubClass (Audio Streaming)
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

		/* Standard AS interface 4, alternate 1 */
    LEN_INTERFACE,        // bLength
    DESC_INTERFACE,        // bDescriptorType (Interface)
    0x04,        // bInterfaceNumber 4
    0x01,        // bAlternateSetting
    0x01,        // bNumEndpoints 1
    USB_DEVICE_CLASS_AUDIO,        // bInterfaceClass (Audio)
    AUDIO_SUBCLASS_AUDIOSTREAMING,        // bInterfaceSubClass (Audio Streaming)
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

		/* Class-spec AS ISO Audio Data endpoint Descriptor */
    0x07,        // bLength
    0x24,        // bDescriptorType (See Next Line)
    0x01,        // bDescriptorSubtype (CS_INTERFACE -> AS_GENERAL)
    0x42,        // bTerminalLink
    0x01,        // bDelay 1
    0x01, 0x00,  // wFormatTag (PCM)

		/* Type I format type Descriptor */
    0x0B,        // bLength
    0x24,        // bDescriptorType (See Next Line)
    0x02,        // bDescriptorSubtype (CS_INTERFACE -> FORMAT_TYPE)
    0x01,        // bFormatType 1
    0x04,        // bNrChannels 4
    0x03,        // bSubFrameSize 3
    0x18,        // bBitResolution 24
    0x01,        // bSamFreqType 1
    B3VAL(REC_RATE),  // tSamFreq[1] 16000 Hz

		/* Standard AS ISO Audio Data Endpoint */
    0x09,        // bLength
    0x05,        // bDescriptorType (See Next Line)
    EP8 | EP_INPUT,        // bEndpointAddress (IN/D2H)
    0x0D,        // bmAttributes (Isochronous, Sync, Data EP)
    WBVAL(EP8_MAX_PKT_SIZE),  // wMaxPacketSize 240
    0x01,        // bInterval 1 (unit depends on device speed)
    0x00,        // bRefresh
    0x00,        // bSyncAddress

		/* Class-spec AS ISO Audio Data endpoint Descriptor */
    0x07,        // bLength
    0x25,        // bDescriptorType (See Next Line)
    0x01,        // bDescriptorSubtype (CS_ENDPOINT -> EP_GENERAL)
    0x00,        // bmAttributes (None)
    0x00,        // bLockDelayUnits
    0x00, 0x00,  // wLockDelay 0
	
    /* Standard AS interface 4, alternate 0 */
    LEN_INTERFACE,                           /* bLength */
    DESC_INTERFACE,                           /* bDescriptorType */
    0x05,                            /* bInterfaceNumber */
    0x00,                           /* bAlternateSetting */
    0x00,                           /* bNumEndpoints */
    USB_DEVICE_CLASS_AUDIO,                           /* bInterfaceClass:AUDIO */
    AUDIO_SUBCLASS_AUDIOSTREAMING,                           /* bInterfaceSubClass:AUDIOSTREAMING */
    0x00,                           /* bInterfaceProtocol */
    0x00,                           /* iInterface */

    /* Standard AS interface 5, alternate 1 */
    0x09,                           /* bLength */
    0x04,                           /* bDescriptorType */
    0x5,                            /* bInterfaceNumber */
    0x01,                           /* bAlternateSetting */
    0x02,                           /* bNumEndpoints */
    USB_DEVICE_CLASS_AUDIO,                           /* bInterfaceClass:AUDIO */
    AUDIO_SUBCLASS_AUDIOSTREAMING,                           /* bInterfaceSubClass:AUDIOSTREAMING */
    0x00,                           /* bInterfaceProtocol */
    0x00,                           /* iInterface */

    /* Class-spec AS interface */
    0x07,                           /* bLength */
    0x24,                           /* bDescriptorType:CS_INTERFACE */
    0x01,                           /* bDescriptorSubType:AS_GENERAL */
    0x01,                           /* bTernimalLink */
    0x01,                           /* bDelay */
		0x01, 0x00,                     /* wFormatTag */ 

    /* Type I format type Descriptor */
    0x0B,                     		/* bLength */
    0x24,                           /* bDescriptorType:CS_INTERFACE */
    0x02,                           /* bDescriptorSubType:FORMAT_TYPE */
    0x01,                           /* bFormatType:FORMAT_TYPE_I */
    0x02,                           /* bNrChannels */
    0x02,                           /* bSubFrameSize */
    0x10,                           /* bBitResolution */
    0x01,                           /* bSamFreqType : 0 continuous; 1 discrete */
		B3VAL(PLAY_RATE_48K),               /* tSamFreq[1] 48000 Hz	*/ 

    /* Standard AS ISO Audio Data Endpoint */
    0x09,                       		/* bLength */
    0x05,                       		/* bDescriptorType */
    EP3 | EP_OUTPUT, 					            /* bEndpointAddress */
		0x15,								/* bmAttributes */ 
    WBVAL(EP3_MAX_PKT_SIZE),                         /* wMaxPacketSize 288 */
    0x01,                       		/* bInterval */
    0x00,                       		/* bRefresh */
    0x84,                       		/* bSynchAddress */

    /* Class-spec AS ISO Audio Data endpoint Descriptor */
		0x07,           					/* bLength */
		0x25,           					/* bDescriptorType:CS_ENDPOINT */
		0x01,           					/* bDescriptorSubType:EP_GENERAL */
		0x01,           					/* bmAttributes */
		0x00,           					/* bLockDelayUnits */
		0x00, 0x00,     					/* wLockDelay */

		/* Standard AS ISO Audio Data Endpoint */
		0x09, 								/* bLength */ 
		0x05,								/* bDescriptorType */
		EP4 | EP_INPUT,   				            /* bEndpointAddress */
		0x11,								/* bmAttributes */
		WBVAL(EP4_MAX_PKT_SIZE),                         /* wMaxPacketSize */
		0x01,								/* bInterval */
		0x05,								/* bRefresh */	  
		0x00,								/* bSynchAddress */
};

/*!<USB Language String Descriptor */
const uint8_t gu8StringLang[4] = 
{
    4,              /* bLength */
    DESC_STRING,    /* bDescriptorType */
    0x09, 0x04
};

/*!<USB Vendor String Descriptor */
const uint8_t gu8VendorStringDesc[16] = 
{
    16,
    DESC_STRING,
    'N', 0, 'u', 0, 'v', 0, 'o', 0, 't', 0, 'o', 0, 'n', 0
};

/*!<USB Product String Descriptor */
const uint8_t gu8ProductStringDesc[] = {
		44,             /* bLength          */
		DESC_STRING,    /* bDescriptorType  */
		'N', 0,
		'u', 0,
		'v', 0,
		'o', 0,
		't', 0,
		'o', 0,
		'n', 0,
		' ', 0,
		'U', 0, 
		'A', 0, 
		'C', 0, 
		' ', 0, 
		'D', 0, 
		'M', 0, 
		'I', 0, 
		'C', 0,
		' ', 0, 
		'D', 0, 
		'P', 0, 
		'W', 0, 
		'M', 0
};


uint8_t gu8StringSerial[26] = 
{
    26,             // bLength
    DESC_STRING,    // bDescriptorType
    'A', 0, '0', 0, '0', 0, '0', 0, '0', 0, '8', 0, '0', 0, '4', 0, '0', 0, '1', 0, '1', 0, '5', 0

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


const uint8_t *gpu8UsbString[4] = {
    gu8StringLang,
    gu8VendorStringDesc,
    gu8ProductStringDesc,
    gu8StringSerial
};

const uint8_t *gu8UsbHidReport[8] = {
    NULL,
    NULL,
    NULL,
    NULL,
	NULL,
    NULL,
    gu8HidReportDesc,
    NULL    
};

const uint32_t gu32UsbHidReportLen[8] = {
    0,
    0,
    0,
    0,
    0,
    0,
    sizeof(gu8HidReportDesc),
    0
};

const uint32_t gu32ConfigHidDescIdx[4] = {
    0,
    0,
    0,
    (sizeof(gu8ConfigDescriptor) - LEN_ENDPOINT - LEN_HID)
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

