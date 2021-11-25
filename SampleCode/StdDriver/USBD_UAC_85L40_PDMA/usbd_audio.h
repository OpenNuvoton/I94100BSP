/**************************************************************************//**
 * @file     usbd_uac.h
 * @version  V1.00
 * @brief    USB driver header file
 *
 * @copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
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
#define UAC_REC_CHANNEL     (4)			// User could select : 1/2/4
#define REC_RATE         		16000   	 
#define AMIC_REC_RATE       	REC_RATE  		
#define AMIC_BIT_RES					16

/* Define Descriptor information */
#define PLAY_FEATURE_UNITID   0x06
#define REC_FEATURE_UNITID    0x05

#if (UAC_REC_CHANNEL == 1)
	#define REC_CH_CFG  0x1
#endif 
#if (UAC_REC_CHANNEL == 2)
	#define REC_CH_CFG  0x3
#endif 
#if (UAC_REC_CHANNEL == 4)
	#define REC_CH_CFG  0xF
#endif 


#if (UAC_REC_CHANNEL == 1)
	#define AMIC2PDMA_BUFF_LEN  	(AMIC_REC_RATE*2*UAC_REC_CHANNEL/1000)  
	#define AMIC2PDMA_BUFF_WC 		((AMIC_REC_RATE*2*UAC_REC_CHANNEL*(AMIC_BIT_RES>>3)/1000)>>2) 		
#else
	#define AMIC2PDMA_BUFF_LEN  	(AMIC_REC_RATE*UAC_REC_CHANNEL/1000)  
	#define AMIC2PDMA_BUFF_WC 		((AMIC_REC_RATE*UAC_REC_CHANNEL*(AMIC_BIT_RES>>3)/1000)>>2) 
#endif 

#define AMIC_RING_BUFFER_LEN  (AMIC2PDMA_BUFF_WC*10)
#define AMIC_BUFF_UPPER_THRE	(AMIC2PDMA_BUFF_WC*7)
#define AMIC_BUFF_LOWER_THRE	(AMIC2PDMA_BUFF_WC*3)
#define AMIC_EP2BC_OFFSET			((AMIC_BIT_RES>>3)*UAC_REC_CHANNEL)


/********************************************/
/* Audio Class Current State                */
/********************************************/
/*!<Define Audio Class Current State */
#define UAC_STOP_AUDIO_RECORD       	0
#define UAC_START_AUDIO_RECORD      	1
#define UAC_PROCESS_AUDIO_RECORD    	2
#define UAC_READY_AUDIO_RECORD     		3
#define UAC_BUSY_AUDIO_RECORD       	4


#define UAC_STOP_AUDIO_SPEAK           0
#define UAC_START_AUDIO_SPEAK          1
#define UAC_PROCESSING_AUDIO_SPEAK     2
#define UAC_BUSY_AUDIO_SPEAK           3


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
