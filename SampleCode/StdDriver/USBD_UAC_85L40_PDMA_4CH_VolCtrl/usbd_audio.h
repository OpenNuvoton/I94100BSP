/**************************************************************************//**
 * @file     usbd_uac.h
 * @version  V1.00
 * @brief     USB driver header file
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
#define PLAY_CHANNELS    	2         		/* Number of channels. */
#define PLAY_RATE      		PLAY_RATE_48K


/* Microphone */
#define REC_RATE         	16000   	  		
#define AMIC_BIT_RES		16
#define AMIC_DATASIZE_BC	(AMIC_BIT_RES>>3)


/* Define Descriptor information */
#define PLAY_FEATURE_UNITID   0x06
//#define REC_FEATURE_UNITID    0x05


#define REC_FEATURE_UNITID_1CH  0x15
#define REC_FEATURE_UNITID_2CH  0x25
#define REC_FEATURE_UNITID_3CH  0x35
#define REC_FEATURE_UNITID_4CH  0x45

#define INTERFACE_NUMBER_1CH    0x1
#define INTERFACE_NUMBER_2CH    0x2
#define INTERFACE_NUMBER_3CH    0x3
#define INTERFACE_NUMBER_4CH    0x4

#define UAC_REC_CHANNEL_1CH     0x1
#define UAC_REC_CHANNEL_2CH     0x2
#define UAC_REC_CHANNEL_3CH     0x3
#define UAC_REC_CHANNEL_4CH     0x4

#define REC_CH_CFG_1CH          0x1
#define REC_CH_CFG_2CH          0x3
#define REC_CH_CFG_3CH          0x7
#define REC_CH_CFG_4CH          0xF


#define AMIC2PDMA_BUFF_LEN_12CH      (REC_RATE*2/1000)  
#define AMIC2PDMA_BUFF_LEN_34CH      (REC_RATE*4/1000)

#define AMIC_RING_BUFFER_LEN_12CH    12*((REC_RATE*2*(AMIC_DATASIZE_BC)/1000)>>2) 
#define AMIC_RING_BUFFER_LEN_34CH    12*((REC_RATE*4*(AMIC_DATASIZE_BC)/1000)>>2) 
#define AMIC_BUFF_UPPER_THRE_12CH	 8*((REC_RATE*2*(AMIC_DATASIZE_BC)/1000)>>2) 
#define AMIC_BUFF_UPPER_THRE_34CH	 8*((REC_RATE*4*(AMIC_DATASIZE_BC)/1000)>>2) 
#define AMIC_BUFF_LOWER_THRE_12CH	 4*((REC_RATE*2*(AMIC_DATASIZE_BC)/1000)>>2) 
#define AMIC_BUFF_LOWER_THRE_34CH	 4*((REC_RATE*4*(AMIC_DATASIZE_BC)/1000)>>2) 

#define EP2BC_BASE              (REC_RATE*UAC_REC_CHANNEL_1CH*AMIC_DATASIZE_BC/1000)	
#define EP3BC_BASE              (REC_RATE*UAC_REC_CHANNEL_2CH*AMIC_DATASIZE_BC/1000)	
#define EP4BC_BASE              (REC_RATE*UAC_REC_CHANNEL_3CH*AMIC_DATASIZE_BC/1000)	
#define EP5BC_BASE              (REC_RATE*UAC_REC_CHANNEL_4CH*AMIC_DATASIZE_BC/1000)	

#define AMIC_EP2BC_OFFSET		2*(UAC_REC_CHANNEL_1CH*AMIC_DATASIZE_BC)
#define AMIC_EP3BC_OFFSET		2*(UAC_REC_CHANNEL_2CH*AMIC_DATASIZE_BC)
#define AMIC_EP4BC_OFFSET		2*(UAC_REC_CHANNEL_3CH*AMIC_DATASIZE_BC)
#define AMIC_EP5BC_OFFSET		3*(UAC_REC_CHANNEL_4CH*AMIC_DATASIZE_BC)


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

typedef struct 
{
	uint8_t g_u8Current_Mic_ChannNum;
	uint8_t g_usbd_UsbAudioState;   
	uint8_t g_u8amic_pdma_bufidx;
	uint16_t g_u16UAC_Buff_ReadIndex;
	uint16_t g_u16UAC_Buff_WriteIndex;
	uint32_t g_u32MICBuffer[2][AMIC2PDMA_BUFF_LEN_34CH];
	uint32_t g_au32UAC_RingBuff[AMIC_RING_BUFFER_LEN_34CH];	
	
	// UAC features variable
	uint8_t g_usbd_RecMute_1CH;         
	uint8_t g_usbd_RecMute_Cur_1CH;         
	int16_t g_usbd_RecVolume_1CH;        
	int16_t g_usbd_RecVolume_Cur_1CH;    
	int16_t g_usbd_RecMaxVolume_1CH;
	int16_t g_usbd_RecMinVolume_1CH;
	int16_t g_usbd_RecResVolume_1CH;

	uint8_t g_usbd_RecMute_2CH;         
	uint8_t g_usbd_RecMute_Cur_2CH;
	int16_t g_usbd_RecVolume_2CH[2];
	int16_t g_usbd_RecVolume_Cur_2CH[2];
	
	int16_t g_usbd_RecMaxVolume_2CH;
	int16_t g_usbd_RecMinVolume_2CH;
	int16_t g_usbd_RecResVolume_2CH;

	uint8_t g_usbd_RecMute_3CH;         
	uint8_t g_usbd_RecMute_Cur_3CH;
	int16_t g_usbd_RecVolume_3CH[3];      
	int16_t g_usbd_RecVolume_Cur_3CH[3];       
	int16_t g_usbd_RecMaxVolume_3CH;
	int16_t g_usbd_RecMinVolume_3CH;
	int16_t g_usbd_RecResVolume_3CH;

	uint8_t g_usbd_RecMute_4CH;         
	uint8_t g_usbd_RecMute_Cur_4CH;         
	int16_t g_usbd_RecVolume_4CH[4];       
	int16_t g_usbd_RecVolume_Cur_4CH[4];    
	int16_t g_usbd_RecMaxVolume_4CH;
	int16_t g_usbd_RecMinVolume_4CH;
	int16_t g_usbd_RecResVolume_4CH; 
	
	uint32_t g_usbd_PlaySampleRate; 
	uint8_t g_usbd_PlayMute;            
	int16_t g_usbd_PlayVolumeL;          
	int16_t g_usbd_PlayVolumeR;         
	int16_t g_usbd_PlayMaxVolume;
	int16_t g_usbd_PlayMinVolume;
	int16_t g_usbd_PlayResVolume;
    
} UAC_REC_T;
extern volatile UAC_REC_T UAC_REC;

#endif //__USBD_UAC_H__

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
