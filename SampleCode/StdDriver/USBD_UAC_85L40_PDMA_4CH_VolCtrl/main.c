/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 19/11/25 11:44a $
 * @brief    USBD Emulation
             Transfer data from USB device to PC through USB HID interface.
 *           After USB enumeration completed, device always IN data to USB host.
 * @note
 * Copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h> 
#include "Platform.h"
#include "usbd_audio.h"
#include "audioclass.h"
#include "ConfigSysClk.h"
#include "BUFCTRL.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void UAC_Init(void);
void UAC_Start(void);
// Microphone (NAU85L40) ===============
void NAU85L40_Init(void);
void NAU85L40_MuteVol_Ctrl(uint8_t ChannelNum);
void I2S_PDMA_Init(uint8_t ChannelNum);
void MIC_Start(void);
void MIC_Stop(void);
// HIRC Trim Functions ===============
void HIRC_AutoTrim_Init(void);
void HIRC_AutoTrim_RefSof(void);

/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables and constants                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
volatile UAC_REC_T UAC_REC = 
{
    .g_u8Current_Mic_ChannNum = 1,
    .g_usbd_UsbAudioState = 0,
    .g_u8amic_pdma_bufidx = 1,
    .g_u16UAC_Buff_ReadIndex = 0,
    .g_u16UAC_Buff_WriteIndex = 0,
    
    .g_usbd_RecMute_1CH       = 0x00,     
    .g_usbd_RecMute_Cur_1CH   = 0x00,     
    .g_usbd_RecVolume_1CH     = 0x1000,   
    .g_usbd_RecVolume_Cur_1CH = 0x1000,  
    .g_usbd_RecMaxVolume_1CH  = 0x7FFF,
    .g_usbd_RecMinVolume_1CH  = 0x57FF,
    .g_usbd_RecResVolume_1CH  = 0x400,    
    .g_usbd_RecMute_2CH       = 0x00,     
    .g_usbd_RecMute_Cur_2CH   = 0x00,  
    .g_usbd_RecVolume_2CH[0]  = 0x1000,   
    .g_usbd_RecVolume_2CH[1]  = 0x1000,   
    .g_usbd_RecVolume_Cur_2CH[0] = 0x1000,   
    .g_usbd_RecVolume_Cur_2CH[1] = 0x1000,   
    .g_usbd_RecMaxVolume_2CH  = 0x7FFF,
    .g_usbd_RecMinVolume_2CH  = 0x57FF,
    .g_usbd_RecResVolume_2CH  = 0x400,
    .g_usbd_RecMute_3CH       = 0x00,     
    .g_usbd_RecMute_Cur_3CH   = 0x00,  
    .g_usbd_RecVolume_3CH[0]  = 0x1000,   
    .g_usbd_RecVolume_3CH[1]  = 0x1000,   
    .g_usbd_RecVolume_3CH[2]  = 0x1000,   
    .g_usbd_RecVolume_Cur_3CH[0] = 0x1000,   
    .g_usbd_RecVolume_Cur_3CH[1] = 0x1000,   
    .g_usbd_RecVolume_Cur_3CH[2] = 0x1000,   
    .g_usbd_RecMaxVolume_3CH  = 0x7FFF,
    .g_usbd_RecMinVolume_3CH  = 0x57FF,
    .g_usbd_RecResVolume_3CH  = 0x400,
    .g_usbd_RecMute_4CH       = 0x00,     
    .g_usbd_RecMute_Cur_4CH   = 0x00,  
    .g_usbd_RecVolume_4CH[0]  = 0x1000,   
    .g_usbd_RecVolume_4CH[1]  = 0x1000,   
    .g_usbd_RecVolume_4CH[2]  = 0x1000,   
    .g_usbd_RecVolume_4CH[3]  = 0x1000,   
    .g_usbd_RecVolume_Cur_4CH[0] = 0x1000,   
    .g_usbd_RecVolume_Cur_4CH[1] = 0x1000,   
    .g_usbd_RecVolume_Cur_4CH[2] = 0x1000,   
    .g_usbd_RecVolume_Cur_4CH[3] = 0x1000,    
    .g_usbd_RecMaxVolume_4CH  = 0x7FFF,
    .g_usbd_RecMinVolume_4CH  = 0x57FF,
    .g_usbd_RecResVolume_4CH  = 0x400, 
    
    .g_usbd_PlaySampleRate = PLAY_RATE_48K, 
    .g_usbd_PlayMute      = 0x01,     
    .g_usbd_PlayVolumeL   = 0x1000,   
    .g_usbd_PlayVolumeR   = 0x1000,   
    .g_usbd_PlayMaxVolume = 0x7FFF,
    .g_usbd_PlayMinVolume = 0x8000,
    .g_usbd_PlayResVolume = 0x400,
};

/*---------------------------------------------------------------------------------------------------------*/
/*  MAIN function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int main(void)
{
	// Init System, peripheral clock and multi-function I/O //
	SYSCLK_INITIATE();
	
	// Enable PDMA clock. 
	CLK_EnableModuleClock(PDMA_MODULE);
	// Reset PDMA module
	SYS_ResetModule(PDMA_RST);
	// Enable PDMA's NVIC
	NVIC_EnableIRQ(PDMA_IRQn);

	// Initiate microphone.
	NAU85L40_Init();
	// I2S PDMA Initiate for Channel 1.
	I2S_PDMA_Init(1);
	// Stop microphone. Wait for UAC to start Microphone.
	MIC_Stop();
	
	HIRC_AutoTrim_Init();
	
	// Initiate UAC to playback audio on PC.
	UAC_Init();
	// Start UAC. 
	UAC_Start();
	
	while(1)
	{
		NAU85L40_MuteVol_Ctrl(UAC_REC.g_u8Current_Mic_ChannNum);
	}
}

// UAC(USBD) = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
#define USBD_UAC_PIN_MASK (SYS_GPB_MFPH_PB13MFP_Msk|SYS_GPB_MFPH_PB14MFP_Msk|SYS_GPB_MFPH_PB15MFP_Msk)
#define USBD_UAC_PIN      (SYS_GPB_MFPH_PB13MFP_USBD_DN|SYS_GPB_MFPH_PB14MFP_USBD_DP|SYS_GPB_MFPH_PB15MFP_USBD_VBUS)

void UAC_Init(void)
{
	// gpio multi-function configuration.
	SYS->GPB_MFPH = (SYS->GPB_MFPH&(~USBD_UAC_PIN_MASK))|USBD_UAC_PIN;
	// Enable USBD module clock.
	CLK_EnableModuleClock(USBD_MODULE);
	// Set USBD clock divid
	CLK_SetModuleClock(USBD_MODULE, CLK_CLKSEL4_USBSEL_HIRC, CLK_CLKDIV0_USBD(1));	
	// Initiate USBD hardware IP and input UAC request for hand-shake.
	USBD_Open(&gsInfo, AUDIO_ClassRequest, (SET_INTERFACE_REQ)AUDIO_SetInterface);
	
	// Set USBD confiure callback function to trim HIRC.
	USBD_SetConfigCallback((SET_CONFIG_CB) HIRC_AutoTrim_RefSof);
	
	// Initiate UAC for endpoint configuration and input input buffer control for UAC controlling.
	AUDIO_Init();
	// Enable USB IRQ
	NVIC_EnableIRQ(USBD_IRQn);
}

void UAC_Start(void)
{
	USBD_Start();
}

// PMDA =========================================================================================================
/* PDMA TDF Bit Field Definitions */
#define PDMA_TDF_TD_F_Pos           PDMA_TDSTS_TDIF0_Pos                         /*!< PDMA TDF: TD_Fx Position */
#define PDMA_TDF_TD_F_Msk           (0xFFFFul << PDMA_TDF_TD_F_Pos)              /*!< PDMA TDF: TD_Fx Mask */
#define PDMA_ABTF_TABORT_F_Pos      PDMA_ABTSTS_ABTIF0_Pos                       /*!< PDMA ABTF: TABORT_Fx Position */
#define PDMA_ABTF_TABORT_F_Msk      (0xFFFFul << PDMA_ABTF_TABORT_F_Pos)         /*!< PDMA ABTF: TABORT_Fx Mask */

void PDMA_IRQHandler(void)
{
	int i;
	uint16_t pdma_buffer_len, ring_buffer_len;
	uint32_t u32Status;
	uint32_t u32PDMA_TDFlag;
	
	// Get interrupt status.
	u32Status = PDMA_GET_INT_STATUS();
	//u32Status = PDMA->INTSTS;
	
	// TDF-PDMA Channel Transfer Done Flag
	u32PDMA_TDFlag = PDMA_GET_TD_STS();
	//u32PDMA_TDFlag = PDMA->TDSTS;
	// Clear transfer done flag.
	PDMA_CLR_TD_FLAG(u32PDMA_TDFlag);
	//PDMA->TDSTS = PDMA_TDF_TD_F_Msk;

	// PDMA Read/Write Target Abort Interrupt Flag
	if (u32Status & PDMA_STATUS_ABTIF) 				
	{ 
		// PDMA Channel 2 Read/Write Target Abort Interrupt Status Flag.
		if (PDMA_GET_ABORT_STS() & PDMA_CH2_MASK)  
		{
			// Clear abort flag.
			PDMA_CLR_ABORT_FLAG(PDMA_GET_ABORT_STS());
			//PDMA->ABTSTS = PDMA_ABTF_TABORT_F_Msk;
		}
	}
	// PDMA Read/Write Target Abort Interrupt Flag
	else if (u32Status & PDMA_STATUS_TDIF)
	{      
		// channel 3 done
		if ( u32PDMA_TDFlag & PDMA_CH3_MASK )
		{
			switch ( UAC_REC.g_u8Current_Mic_ChannNum )
			{
				case UAC_REC_CHANNEL_1CH:
				case UAC_REC_CHANNEL_2CH:
					pdma_buffer_len = AMIC2PDMA_BUFF_LEN_12CH;
					ring_buffer_len = AMIC_RING_BUFFER_LEN_12CH;
				break;
				
				case UAC_REC_CHANNEL_3CH:
				case UAC_REC_CHANNEL_4CH:
					pdma_buffer_len = AMIC2PDMA_BUFF_LEN_34CH;
					ring_buffer_len = AMIC_RING_BUFFER_LEN_34CH;
				break;
			}
            
			if (UAC_REC.g_usbd_UsbAudioState == UAC_START_AUDIO_RECORD)
			{		
				for ( i= 0; i < (pdma_buffer_len/2); i++)
				{
					UAC_REC.g_au32UAC_RingBuff[UAC_REC.g_u16UAC_Buff_WriteIndex++] = UAC_REC.g_u32MICBuffer[0][i];
				}	
				UAC_REC.g_usbd_UsbAudioState = UAC_PROCESS_AUDIO_RECORD;
			}
			else if (UAC_REC.g_usbd_UsbAudioState == UAC_PROCESS_AUDIO_RECORD)
			{
				for ( i= 0 ; i < (pdma_buffer_len/2) ; i++)
				{
					UAC_REC.g_au32UAC_RingBuff[UAC_REC.g_u16UAC_Buff_WriteIndex++] = UAC_REC.g_u32MICBuffer[1][i];
				}
				UAC_REC.g_usbd_UsbAudioState = UAC_READY_AUDIO_RECORD;
			}
			else if (UAC_REC.g_usbd_UsbAudioState == UAC_READY_AUDIO_RECORD)
			{
				for ( i= 0 ; i < (pdma_buffer_len/2) ; i++)
				{
					UAC_REC.g_au32UAC_RingBuff[UAC_REC.g_u16UAC_Buff_WriteIndex++] = UAC_REC.g_u32MICBuffer[0][i];
				}
				UAC_REC.g_usbd_UsbAudioState = UAC_BUSY_AUDIO_RECORD;
				UAC_REC.g_u8amic_pdma_bufidx = 1;
			}
			else if (UAC_REC.g_usbd_UsbAudioState == UAC_BUSY_AUDIO_RECORD)
			{
				for ( i= 0 ; i < (pdma_buffer_len/2) ; i++)
				{
					UAC_REC.g_au32UAC_RingBuff[UAC_REC.g_u16UAC_Buff_WriteIndex++] = UAC_REC.g_u32MICBuffer[UAC_REC.g_u8amic_pdma_bufidx][i];
				
					if ( UAC_REC.g_u16UAC_Buff_WriteIndex == ring_buffer_len ) 
						UAC_REC.g_u16UAC_Buff_WriteIndex = 0;       
				}
				
				UAC_REC.g_u8amic_pdma_bufidx ^= 0x1;
			}
		}
	}
}
/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
