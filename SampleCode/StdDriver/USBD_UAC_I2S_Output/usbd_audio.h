/**************************************************************************//**
 * @file     usbd_uac.h
 * @version  V1.00
 * @brief    USB driver header file
 *
 * @copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef __USBD_UAC_H__
#define __USBD_UAC_H__


/* Audio Interface Subclass Codes */
#define AUDIO_SUBCLASS_UNDEFINED               0x00
#define AUDIO_SUBCLASS_AUDIOCONTROL            0x01
#define AUDIO_SUBCLASS_AUDIOSTREAMING          0x02
#define AUDIO_SUBCLASS_MIDISTREAMING           0x03

/* Audio Interface Protocol Codes */
#define AUDIO_PROTOCOL_UNDEFINED               0x00


/* bmAttributes in Configuration Descriptor */
#define USB_CONFIG_POWERED_MASK                0x40
#define USB_CONFIG_BUS_POWERED                 0x80
#define USB_CONFIG_SELF_POWERED                0xC0
#define USB_CONFIG_REMOTE_WAKEUP               0x20

/* bMaxPower in Configuration Descriptor */
#define USB_CONFIG_POWER_MA(mA)                ((mA)/2)


/*!<Define Audio information */
#define PLAY_RATE_48K       48000     		/* The audo play sampling rate. */
#define PLAY_RATE_96K       96000     		
#define PLAY_CHANNELS    		2         		/* Number of channels. */
#define PLAY_RATE      			PLAY_RATE_48K


/* Microphone */
//#define REC_MODE_MONO		
//#define REC_MODE_2CH
#define REC_MODE_4CH
//#define REC_RATE         		16000    /* The record sampling rate. Must be the same with PLAY_RATE */


// Unit ID  
#define PLAY_FEATURE_UNITID   0x06
#define REC_FEATURE_UNITID    0x05


/* Define Descriptor information */
#if(PLAY_CHANNELS == 1)
    #define PLAY_CH_CFG     1
#endif
#if(PLAY_CHANNELS == 2)
    #define PLAY_CH_CFG     3
#endif

#if(REC_CHANNELS == 1)
    #define REC_CH_CFG     0x1
#endif
#if(REC_CHANNELS == 2)
    #define REC_CH_CFG     0x3
#endif
#if(REC_CHANNELS == 4)
    #define REC_CH_CFG     0xF
#endif

#define PLAY_RATE_LO    (PLAY_RATE & 0xFF)
#define PLAY_RATE_MD    ((PLAY_RATE >> 8) & 0xFF)
#define PLAY_RATE_HI    ((PLAY_RATE >> 16) & 0xFF)

#define MAX_USB_BUFFER_LEN  	(10*PLAY_RATE/1000)		
#define USB_BUFFER_THRE_BASE    (PLAY_RATE/1000)			
#define USB_BUFF_UPPER_THRE  	(USB_BUFFER_THRE_BASE*7)		 
#define USB_BUFF_LOWER_THRE  	(USB_BUFFER_THRE_BASE*4)		 
#define PDMA2I2S_BUFF_LEN  	(PLAY_RATE*PLAY_CHANNELS/1000)  
#define USB_BUFFER_EP4_BASE		(0xC0000)
#define USB_BUFFER_EP4_OFFSET	(0x0C000)
#define USB_BUFFER_EP4_ADD		(0x0C000)
#define USB_BUFFER_EP4_SUB		(0x08000)



/********************************************/
/* Audio Class Current State                */
/********************************************/
/*!<Define Audio Class Current State */
#define UAC_STOP_AUDIO_SPEAK            0
#define UAC_START_AUDIO_SPEAK           1
#define UAC_PROCESS1_AUDIO_SPEAK     	2
#define UAC_PROCESS2_AUDIO_SPEAK     	3
#define UAC_BUSY_AUDIO_SPEAK            4


/***************************************************/
/*      Audio Class-Specific Request Codes         */
/***************************************************/
/*!<Define Audio Class Specific Request */
#define UAC_REQUEST_CODE_UNDEFINED  0x00
#define UAC_SET_CUR                 0x01
#define UAC_GET_CUR                 0x81
#define UAC_SET_MIN                 0x02
#define UAC_GET_MIN                 0x82
#define UAC_SET_MAX                 0x03
#define UAC_GET_MAX                 0x83
#define UAC_SET_RES                 0x04
#define UAC_GET_RES                 0x84
#define UAC_SET_MEM                 0x05
#define UAC_GET_MEM                 0x85
#define UAC_GET_STAT                0xFF

/*!<Define HID Class Specific Request */
#define HID_SET_REPORT              0x09
#define HID_SET_IDLE                0x0A
#define HID_SET_PROTOCOL            0x0B

#define MUTE_CONTROL                0x01
#define VOLUME_CONTROL              0x02

//typedef struct _usb_fb_ept_t
//{
//	uint16_t count;
//	uint16_t buffer[512];			
//} usb_fb_ept_t;

//extern volatile usb_fb_ept_t usb_fb_ept;

#endif //__USBD_UAC_H__

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
