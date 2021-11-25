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
#define PLAY_CHANNELS    	2         		/* Number of channels. */
#define PLAY_RATE      		PLAY_RATE_48K


/* Microphone */
#define REC_RATE         	16000    /* The record sampling rate. Must be the same with PLAY_RATE */
#define DMIC_REC_RATE       REC_RATE  		
#define DMIC_BIT_RES		(24)

#define REC_CHANNELNUM_1CH      0x1           
#define REC_CHANNELNUM_2CH      0x2           
#define REC_CHANNELNUM_3CH      0x3
#define REC_CHANNELNUM_4CH      0x4

// Unit ID  
#define PLAY_INPUT_ID   	   0x01
#define PLAY_OUTPUT_ID   	   0x03
#define PLAY_FEATURE_UNITID    0x05
#define REC1CH_OUTPUT_ID 	   0x12
#define REC1CH_INPUT_ID 	   0x14
#define REC1CH_FEATURE_UNITID  0x16
#define REC2CH_OUTPUT_ID 	   0x22
#define REC2CH_INPUT_ID 	   0x24
#define REC2CH_FEATURE_UNITID  0x26
#define REC3CH_OUTPUT_ID 	   0x32
#define REC3CH_INPUT_ID 	   0x34
#define REC3CH_FEATURE_UNITID  0x36
#define REC4CH_OUTPUT_ID 	   0x42
#define REC4CH_INPUT_ID 	   0x44
#define REC4CH_FEATURE_UNITID  0x46



/* Define Descriptor information */
#if(PLAY_CHANNELS == 1)
    #define PLAY_CH_CFG     1
#endif
#if(PLAY_CHANNELS == 2)
    #define PLAY_CH_CFG     3
#endif
    
#define UAC_AC_IFNUM      0x0
#define REC_IFNUM_1CH     0x1    
#define REC_IFNUM_2CH     0x2
#define REC_IFNUM_3CH     0x3
#define REC_IFNUM_4CH     0x4
#define PLAY_IFNUM        0x5
//#define HID_IFNUM        0x6


#define PLAY_RATE_LO    (PLAY_RATE & 0xFF)
#define PLAY_RATE_MD    ((PLAY_RATE >> 8) & 0xFF)
#define PLAY_RATE_HI    ((PLAY_RATE >> 16) & 0xFF)

#define REC_RATE_LO     (REC_RATE & 0xFF)
#define REC_RATE_MD     ((REC_RATE >> 8) & 0xFF)
#define REC_RATE_HI     ((REC_RATE >> 16) & 0xFF)


#define MAX_USB_BUFFER_LEN  	(10*PLAY_RATE/1000)		
#define USB_BUFFER_THRE_BASE    (PLAY_RATE/1000)			
#define USB_BUFF_UPPER_THRE  	(USB_BUFFER_THRE_BASE*7)		 
#define USB_BUFF_LOWER_THRE  	(USB_BUFFER_THRE_BASE*4)		 
#define PDMA2DPWM_BUFF_LEN  	(PLAY_RATE*PLAY_CHANNELS/1000)  
//#define DMIC2PDMA_BUFF_LEN  	(REC_RATE*REC_CHANNELS/1000)
#define DMIC2PDMA_BUFF_LEN  	(REC_RATE*REC_CHANNELNUM_4CH/1000)
#define USB_BUFFER_EP4_BASE		(0xC0000)
#define USB_BUFFER_EP4_OFFSET	(0x0C000)
#define USB_BUFFER_EP4_ADD		(0x0C000)
#define USB_BUFFER_EP4_SUB		(0x08000)


#define DMIC_RING_BUFFER_LEN_1CH    (30*REC_RATE*REC_CHANNELNUM_1CH/1000)
#define DMIC_RING_BUFFER_LEN_2CH    (30*REC_RATE*REC_CHANNELNUM_2CH/1000)
#define DMIC_RING_BUFFER_LEN_3CH    (30*REC_RATE*REC_CHANNELNUM_3CH/1000)
#define DMIC_RING_BUFFER_LEN_4CH    (30*REC_RATE*REC_CHANNELNUM_4CH/1000)

#define DMIC_BUFF_UPPER_THRE_1CH    (20*REC_RATE*REC_CHANNELNUM_1CH/1000)   
#define DMIC_BUFF_UPPER_THRE_2CH    (20*REC_RATE*REC_CHANNELNUM_2CH/1000)  
#define DMIC_BUFF_UPPER_THRE_3CH    (20*REC_RATE*REC_CHANNELNUM_3CH/1000)
#define DMIC_BUFF_UPPER_THRE_4CH    (20*REC_RATE*REC_CHANNELNUM_4CH/1000)

#define DMIC_BUFF_LOWER_THRE_1CH    (10*REC_RATE*REC_CHANNELNUM_1CH/1000)   
#define DMIC_BUFF_LOWER_THRE_2CH    (10*REC_RATE*REC_CHANNELNUM_2CH/1000)  
#define DMIC_BUFF_LOWER_THRE_3CH    (10*REC_RATE*REC_CHANNELNUM_3CH/1000)
#define DMIC_BUFF_LOWER_THRE_4CH    (10*REC_RATE*REC_CHANNELNUM_4CH/1000)

#define DMIC_EP2BC_BASE             (REC_RATE*REC_CHANNELNUM_1CH*3/1000)
#define DMIC_EP2BC_ADD              (6)   
#define DMIC_EP2BC_SUB              (6)   

#define DMIC_EP6BC_BASE             (REC_RATE*REC_CHANNELNUM_2CH*3/1000)
#define DMIC_EP6BC_ADD              (12)   
#define DMIC_EP6BC_SUB              (12)   

#define DMIC_EP7BC_BASE             (REC_RATE*REC_CHANNELNUM_3CH*3/1000)
#define DMIC_EP7BC_ADD              (18)   
#define DMIC_EP7BC_SUB              (18)   

#define DMIC_EP8BC_BASE             (REC_RATE*REC_CHANNELNUM_4CH*3/1000)
#define DMIC_EP8BC_ADD              (24)   
#define DMIC_EP8BC_SUB              (24)   

/********************************************/
/* Audio Class Current State                */
/********************************************/
/*!<Define Audio Class Current State */
#define UAC_STOP_AUDIO_RECORD           0
#define UAC_START_AUDIO_RECORD          1
#define UAC_PROCESS_AUDIO_RECORD        2
#define UAC_READY_AUDIO_RECORD          3
#define UAC_BUSY_AUDIO_RECORD           4

#define UAC_STOP_AUDIO_SPEAK            0
#define UAC_START_AUDIO_SPEAK           1
#define UAC_PROCESS1_AUDIO_SPEAK        2
#define UAC_PROCESS2_AUDIO_SPEAK        3
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


#endif //__USBD_UAC_H__

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
