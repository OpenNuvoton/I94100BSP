//----------------------------------------------------------------------------
// @file     main.c
// @version  V1.00
// $Revision: 1 $
// $Date: 16/06/14 9:36a $
// @brief    Please refer readme.txt
// @note
// Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
//----------------------------------------------------------------------------
#include <stdio.h>
#include <string.h> 
#include "Platform.h"
#include "usbd_audio.h"
#include "audioclass.h"
#include "ConfigSysClk.h"
#include "pdma_config.h"
#include "uac_buffer.h"

//----------------------------------------------------------------------------
//  Constant Define
//----------------------------------------------------------------------------
#define DMIC_GROUP_1
#define DPWM_GROUP_1
#define DMIC_PDMA_32BIT
//#define DPWM_PDMA_32BIT
#define DPWM_PDMA_16BIT

//----------------------------------------------------------------------------
//  Playback Buffer Control Variable
//----------------------------------------------------------------------------
// Extern Variable Declaration
// Buffer control.
extern volatile uint32_t g_au32PDMA2USB_RingBuff[];
extern volatile uint8_t g_u8DPWM_Buff_Index;
extern volatile uint32_t g_au32USB2PDMA_Buff[MAX_USB_BUFFER_LEN];
extern volatile uint32_t g_au32PDMA2DPWM_Buff[2][MAX_USB_BUFFER_LEN];

extern volatile uint16_t g_u16PlayBack_Read_Ptr;
extern volatile uint16_t g_u16PlayBack_Write_Ptr;
// UAC State Control structure.
extern UAC_PLAYBACK_T UAC_PLAYBACK;
extern UAC_RECODER_T UAC_RECODER;

// Global Variable Declaration
volatile uint16_t g_u16PDMA2DPWM_Bufflen = 96;

//----------------------------------------------------------------------------
//  PDMA Structure Define
//----------------------------------------------------------------------------
// Scatter-Gather
DMA_DESC_T DMA_TXDESC[2], DMA_RXDESC[2];
// Control
PDMA_CONFIG_T PDMA_CONFIG;

//----------------------------------------------------------------------------
//  Functions Definition
//----------------------------------------------------------------------------
void USBD_Init(void);
void UART0_Init(void);
void PDMA_Init(PDMA_CONFIG_T *pdma_ctl);
void DPWM_Init(uint32_t u32SampleRate);
void DMIC_Init(uint32_t u32SampleRate, uint8_t ChannelNum);
void HIRC_AutoTrim_Init(void);
void HIRC_AutoTrim_RefSof(void);

//----------------------------------------------------------------------------
//  MAIN function
//----------------------------------------------------------------------------
int main(void)
{
	
	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();
	UART0_Init();
	
	// Config PDMA transfer count and status.
	PDMA_CONFIG.play_en = 1;
	PDMA_CONFIG.play_buflen = PLAY_RATE_48K*PLAY_CHANNELS/1000;
	PDMA_CONFIG.rec_en  = 1;
	PDMA_CONFIG.rec_buflen = REC_RATE*REC_CHANNELNUM_1CH/1000;
	PDMA_Init(&PDMA_CONFIG);
	
	// Enable PDMA engine clock.
	CLK_EnableModuleClock(PDMA_MODULE);
	// Reset PDMA module.
	SYS_ResetModule(PDMA_RST);

#if (SYSCLK_HCLK_CLK == SYSCLK_HCLK_CLK_PLL_HIRC)
	HIRC_AutoTrim_Init();
#endif
	
	USBD_Init();
	USBD_Start();

	while(1);
}

//----------------------------------------------------------------------------
//  PDMA IRQ Handler function
//----------------------------------------------------------------------------
void PDMA_IRQHandler(void)
{
	int i;
	uint16_t u16dmic2pdmabuflen, u16usbmaxbuflen; 
	uint32_t u32Status;
	uint32_t u32Transferdone;
	
	// Get interrupt status.
	u32Status = PDMA_GET_INT_STATUS();
	
	// TDF-PDMA Channel Transfer Done Flag
	u32Transferdone = PDMA_GET_TD_STS();
	// write 1 to clear
	PDMA_CLR_TD_FLAG(u32Transferdone);

	// PDMA Read/Write Target Abort Interrupt Flag
	if(u32Status & PDMA_STATUS_ABTIF)
	{
		//PDMA Channel 2 Read/Write Target Abort Interrupt Status Flag
		if (PDMA_GET_ABORT_STS() & BIT2)  
		{
			PDMA_CLR_ABORT_FLAG(BIT2);
		}
	}
	else if(u32Status & PDMA_STATUS_TDIF) 	// Transfer Done Interrupt Flag
	{
		// channel 2 done 
		if(u32Transferdone & BIT2 )		
		{
			// Depend on how many channel is active to decide buffer size.
			switch (UAC_RECODER.u8AudioRecInterfConfig)
			{
				case REC_IFNUM_1CH:
				u16dmic2pdmabuflen = REC_RATE*REC_CHANNELNUM_1CH/1000;
				u16usbmaxbuflen = DMIC_RING_BUFFER_LEN_1CH;
				break;
				case REC_IFNUM_2CH:
				u16dmic2pdmabuflen = REC_RATE*REC_CHANNELNUM_2CH/1000;
				u16usbmaxbuflen = DMIC_RING_BUFFER_LEN_2CH;
				break;
				case REC_IFNUM_3CH:
				u16dmic2pdmabuflen = REC_RATE*REC_CHANNELNUM_3CH/1000;
				u16usbmaxbuflen = DMIC_RING_BUFFER_LEN_3CH;
				break;
				default:
				case REC_IFNUM_4CH:
				u16dmic2pdmabuflen = REC_RATE*REC_CHANNELNUM_4CH/1000;
				u16usbmaxbuflen = DMIC_RING_BUFFER_LEN_4CH;
				break;
			}
			
			if (UAC_RECODER.u8AudioMicState == UAC_START_AUDIO_RECORD)
			{
				for (i = 0 ; i < u16dmic2pdmabuflen ; i++)
				{
					g_au32PDMA2USB_RingBuff[UAC_RECODER.u16UAC_Buff_WriteIndex++] = UAC_RECODER.au32DMIC2PDMA_Buff[0][i]; 
				}
				
				UAC_RECODER.u8AudioMicState = UAC_PROCESS_AUDIO_RECORD;
			}
			else if (UAC_RECODER.u8AudioMicState == UAC_PROCESS_AUDIO_RECORD)
			{
				for (i = 0 ; i < u16dmic2pdmabuflen ; i++)
				{
					g_au32PDMA2USB_RingBuff[UAC_RECODER.u16UAC_Buff_WriteIndex++] = UAC_RECODER.au32DMIC2PDMA_Buff[1][i]; 
				}
					
				UAC_RECODER.u8AudioMicState = UAC_READY_AUDIO_RECORD;
			}
			else if (UAC_RECODER.u8AudioMicState == UAC_READY_AUDIO_RECORD)
			{
				for (i = 0 ; i < u16dmic2pdmabuflen ; i++)
				{
					g_au32PDMA2USB_RingBuff[UAC_RECODER.u16UAC_Buff_WriteIndex++] = UAC_RECODER.au32DMIC2PDMA_Buff[0][i]; 
				}
				
				UAC_RECODER.u8DMIC_Buff_Index = 0x1;
				UAC_RECODER.u8AudioMicState = UAC_BUSY_AUDIO_RECORD;	
			}
			// Renew the data from record buffer to PDMA buffer
			else if (UAC_RECODER.u8AudioMicState == UAC_BUSY_AUDIO_RECORD)
			{
				for (i = 0 ; i < u16dmic2pdmabuflen ; i++)
				{
					g_au32PDMA2USB_RingBuff[UAC_RECODER.u16UAC_Buff_WriteIndex++] = UAC_RECODER.au32DMIC2PDMA_Buff[UAC_RECODER.u8DMIC_Buff_Index][i]; 
					
					if (UAC_RECODER.u16UAC_Buff_WriteIndex == u16usbmaxbuflen) 
					{
							UAC_RECODER.u16UAC_Buff_WriteIndex = 0;
					}
				}
				
				// Switch to another buffer row.
				UAC_RECODER.u8DMIC_Buff_Index ^= 0x1;    
			}
		}
		
		// channel 1 done 
		if(u32Transferdone & BIT1)
		{
			for ( i = 0 ; i < g_u16PDMA2DPWM_Bufflen ; i++)
			{
				if(g_u16PlayBack_Read_Ptr == g_u16PlayBack_Write_Ptr)
				{
					if(g_u16PlayBack_Read_Ptr != 0)
						g_u16PlayBack_Read_Ptr--;
					else
						g_u16PlayBack_Read_Ptr = MAX_USB_BUFFER_LEN-1;
				}
				
				if (UAC_PLAYBACK.usbd_PlayMute)
				{
					g_au32PDMA2DPWM_Buff[g_u8DPWM_Buff_Index][i] = 0x0;
					g_u16PlayBack_Read_Ptr++;
				}
				else
				{
					g_au32PDMA2DPWM_Buff[g_u8DPWM_Buff_Index][i] = g_au32USB2PDMA_Buff[g_u16PlayBack_Read_Ptr];
					g_u16PlayBack_Read_Ptr++;
				}
				
				if ( g_u16PlayBack_Read_Ptr == MAX_USB_BUFFER_LEN )
					g_u16PlayBack_Read_Ptr = 0;
			}
			// switch to another buffer row.
			g_u8DPWM_Buff_Index ^= 0x1;
		}
	}
}

//----------------------------------------------------------------------------
//  HIRC Trim function
//----------------------------------------------------------------------------
void IRC_IRQHandler(void)
{
	// Get Trim Failure Interrupt
	if(SYS_GET_TRIMHIRC_INT_FLAG(SYS_IRCTISTS_TRIMFAIL_INT_FLAG))
	{ 
		// Clear Trim Failure Interrupt
		SYS_CLEAR_TRIMHIRC_INT_FLAG(SYS_IRCTISTS_TRIMFAIL_INT_FLAG);
	}
	
	// Get LXT Clock Error Interrupt
	if(SYS_GET_TRIMHIRC_INT_FLAG(SYS_IRCTISTS_CLKERROR_INT_FLAG)) 
	{ 
		// Clear LXT Clock Error Interrupt 
		SYS_CLEAR_TRIMHIRC_INT_FLAG(SYS_IRCTISTS_CLKERROR_INT_FLAG);
	}
}

void TMR0_IRQHandler(void)
{
	static uint8_t u8Count = 0;

	if(TIMER_GetIntFlag(TIMER0) == 1)
	{
		/* Clear Timer0 time-out interrupt flag */
		TIMER_ClearIntFlag(TIMER0);
	
		if(++u8Count >= 10)
		{
			SYS_EnableTrimHIRC(SYS_IRCTCTL_FREQSEL_48M);
			u8Count = 0;
		}
	}
}

void HIRC_AutoTrim_Init(void)
{
	SYS_SET_TRIMHIRC_LOOPSEL(SYS_IRCTCTL_LOOPSEL_4);
	SYS_SET_TRIMHIRC_RETRYCNT(SYS_IRCTCTL_RETRYCNT_64);
	SYS_ENABLE_TRIMHIRC_CLKERRSTOP();
	SYS_SET_TRIMHIRC_REFCLK(SYS_IRCTCTL_REFCLK_USBSOF);
	
	// Enable clock error / trim fail interrupt 		
	SYS_ENABLE_TRIMHIRC_INT(SYS_IRCTIEN_TRIMFAIL_INT_MASK|SYS_IRCTIEN_CLKERROR_INT_MASK);

	NVIC_EnableIRQ(IRC_IRQn);
	
	// Timer Initiate for periodic HIRC Auto Trim.
	/* Enable peripheral clock */
	CLK_EnableModuleClock(TMR0_MODULE);
	/* Peripheral clock source */
	CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_PCLK0, 0);
	
	/* Open Timer0 in periodic mode, enable interrupt and 1 interrupt tick per second */
	TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 1);
	TIMER_EnableInt(TIMER0);
	
	/* Enable Timer0 ~ Timer3 NVIC */
	NVIC_EnableIRQ(TMR0_IRQn);
	
	/* Start Timer0 ~ Timer3 counting */
	TIMER_Start(TIMER0);
}

void HIRC_AutoTrim_RefSof(void)
{																													
	// HIRC auto trim enable/disable 
	SYS_EnableTrimHIRC(SYS_IRCTCTL_FREQSEL_48M);

	while(!SYS_IS_TRIMHIRC_DONE())
	{
	}
	
	SYS_CLEAR_TRIMHIRC_INT_FLAG(SYS_IRCTISTS_TRIMFAIL_INT_FLAG|SYS_IRCTISTS_CLKERROR_INT_FLAG);
}

//----------------------------------------------------------------------------
//  USBD Initial function
//----------------------------------------------------------------------------
void USBD_Init(void)
{
	// MFP select 
	SYS->GPB_MFPH &= ~( SYS_GPB_MFPH_PB13MFP_Msk | SYS_GPB_MFPH_PB14MFP_Msk | SYS_GPB_MFPH_PB15MFP_Msk );
	SYS->GPB_MFPH |= SYS_GPB_MFPH_PB13MFP_USBD_DN | SYS_GPB_MFPH_PB14MFP_USBD_DP | SYS_GPB_MFPH_PB15MFP_USBD_VBUS;

	// Select IP clock source
	CLK_EnableModuleClock(USBD_MODULE);
	CLK_SetModuleClock(USBD_MODULE, CLK_CLKSEL4_USBSEL_HIRC, CLK_CLKDIV0_USBD(1));
	
	// USBD initial.
	USBD_Open(&gsInfo, AUDIO_ClassRequest, (SET_INTERFACE_REQ)AUDIO_SetInterface);
	
#if (SYSCLK_HCLK_CLK == SYSCLK_HCLK_CLK_PLL_HIRC)
	USBD_SetConfigCallback((SET_CONFIG_CB) HIRC_AutoTrim_RefSof);
#endif
	
	// Endpoint configuration.
	AUDIO_Init();
	NVIC_EnableIRQ(USBD_IRQn);
}
//----------------------------------------------------------------------------
//  DMIC Initial function
//----------------------------------------------------------------------------
void DMIC_Init(uint32_t u32SampleRate, uint8_t ChannelNum)
{
#ifdef DMIC_GROUP_1
	SYS->GPD_MFPL &= ~(SYS_GPD_MFPL_PD3MFP_Msk | SYS_GPD_MFPL_PD4MFP_Msk | SYS_GPD_MFPL_PD5MFP_Msk | SYS_GPD_MFPL_PD6MFP_Msk);
	SYS->GPD_MFPL |= (SYS_GPD_MFPL_PD3MFP_DMIC_CLK1 | SYS_GPD_MFPL_PD4MFP_DMIC_DAT1 | SYS_GPD_MFPL_PD5MFP_DMIC_CLK0 | SYS_GPD_MFPL_PD6MFP_DMIC_DAT0);
#endif
	
	// DMIC_CLK Select.
	CLK_SetModuleClock(DMIC_MODULE, CLK_CLKSEL2_DMICSEL_PLL, NULL);

	// Enable DMIC clock.
	CLK_EnableModuleClock(DMIC_MODULE);
	
	// DPWM IP Reset.
	SYS_ResetModule(DMIC_RST);
	
	// Set Down Sample
	DMIC_ENABLE_DOWMSAMPLE(DMIC, DMIC_CTL_DOWNSAMPLE_100_50);

	// Set Sample rate
	DMIC_SetSampleRate(DMIC, u32SampleRate);
	
	// Set data latch 
	DMIC_SET_LATCHDATA(DMIC, DMIC_CTL_LATCHDATA_CH23R|DMIC_CTL_LATCHDATA_CH01R);
	// FIFO threshold
	DMIC_ENABLE_FIFOTHRESHOLDINT(DMIC, 8);
	
	DMIC_ENABLE_PDMA(DMIC);

	DMIC_DISABLE_CHANNEL(DMIC, DMIC_CTL_CH0|DMIC_CTL_CH1|DMIC_CTL_CH2|DMIC_CTL_CH3);
    
	// ch1 - ch0 - ch3 - ch2
	switch (ChannelNum)
	{
		case 0x1:
			DMIC_ENABLE_CHANNEL(DMIC, DMIC_CTL_CH1);
		break;
		case 0x2:
			DMIC_ENABLE_CHANNEL(DMIC, DMIC_CTL_CH1|DMIC_CTL_CH0);
		break;
		case 0x3:
			DMIC_ENABLE_CHANNEL(DMIC, DMIC_CTL_CH1|DMIC_CTL_CH0|DMIC_CTL_CH3);	
		break;
		case 0x4:
			DMIC_ENABLE_CHANNEL(DMIC, DMIC_CTL_CH1|DMIC_CTL_CH0|DMIC_CTL_CH3|DMIC_CTL_CH2);
		break;
	}
}
//----------------------------------------------------------------------------
//  PDMA Initial function
//----------------------------------------------------------------------------
void PDMA_Init(PDMA_CONFIG_T *pdma_ctl)
{
	// PDMA2DPWM_Buffer ->PDMA_CH1->DPWM
	if (pdma_ctl->play_en)
	{
		PDMA_Open(PDMA_CH1_MASK);
		// USB_EP3 to PDMA to DPWM.
		#ifdef DPWM_PDMA_32BIT
		// Tx description.
		DMA_TXDESC[0].ctl = (((pdma_ctl->play_buflen)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		
																											PDMA_WIDTH_32							|		
																											PDMA_SAR_INC							|		
																											PDMA_DAR_FIX							|		
																											PDMA_REQ_SINGLE						|		
																											PDMA_OP_SCATTER;

		DMA_TXDESC[1].ctl = (((pdma_ctl->play_buflen)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		
																											PDMA_WIDTH_32							|		
																											PDMA_SAR_INC							|		
																											PDMA_DAR_FIX							|		
																											PDMA_REQ_SINGLE						|		
																											PDMA_OP_SCATTER;
		#endif
		#ifdef DPWM_PDMA_16BIT
		// Tx description.
		DMA_TXDESC[0].ctl = (((pdma_ctl->play_buflen)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		 
																											PDMA_WIDTH_16							|		
																											PDMA_SAR_INC							|		
																											PDMA_DAR_FIX							|		
																											PDMA_REQ_SINGLE						|		
																											PDMA_OP_SCATTER;			
		
		DMA_TXDESC[1].ctl = (((pdma_ctl->play_buflen)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		 
																											PDMA_WIDTH_16							|		
																											PDMA_SAR_INC							|		
																											PDMA_DAR_FIX							|		
																											PDMA_REQ_SINGLE						|		
																											PDMA_OP_SCATTER;
		#endif
		
		DMA_TXDESC[0].endsrc = (uint32_t)&g_au32PDMA2DPWM_Buff[0][0];    
		DMA_TXDESC[0].enddest = (uint32_t)&DPWM->FIFO;
		DMA_TXDESC[0].offset = ((uint32_t)&DMA_TXDESC[1] - (PDMA->SCATBA));
		
		DMA_TXDESC[1].endsrc = (uint32_t)&g_au32PDMA2DPWM_Buff[1][0];    
		DMA_TXDESC[1].enddest = (uint32_t)&DPWM->FIFO;
		DMA_TXDESC[1].offset = ((uint32_t)&DMA_TXDESC[0] - (PDMA->SCATBA));

		// Request source is memory to memory
		PDMA_SetTransferMode(1, PDMA_DPWM_TX, TRUE, (uint32_t)&DMA_TXDESC[1]);
		// Enable PDMA channel 1 interrupt
		PDMA_EnableInt(1, PDMA_INT_TRANS_DONE);
	}
	// DMIC->PDMA_CH2->DMIC2PDMA_Buffer
	if (pdma_ctl->rec_en)
	{
		PDMA_Open(PDMA_CH2_MASK);
		#ifdef DMIC_PDMA_32BIT
		// Rx description
		DMA_RXDESC[0].ctl = ((pdma_ctl->rec_buflen-1)<<PDMA_DSCT_CTL_TXCNT_Pos)|PDMA_WIDTH_32|PDMA_SAR_FIX|PDMA_DAR_INC|PDMA_REQ_SINGLE|PDMA_OP_SCATTER;
		DMA_RXDESC[0].endsrc = (uint32_t)&DMIC->FIFO;
		DMA_RXDESC[0].enddest = (uint32_t)&UAC_RECODER.au32DMIC2PDMA_Buff[0];  //start address
		DMA_RXDESC[0].offset = (uint32_t)&DMA_RXDESC[1] - (PDMA->SCATBA);

		DMA_RXDESC[1].ctl = ((pdma_ctl->rec_buflen-1)<<PDMA_DSCT_CTL_TXCNT_Pos)|PDMA_WIDTH_32|PDMA_SAR_FIX|PDMA_DAR_INC|PDMA_REQ_SINGLE|PDMA_OP_SCATTER;
		DMA_RXDESC[1].endsrc = (uint32_t)&DMIC->FIFO;
		DMA_RXDESC[1].enddest = (uint32_t)&UAC_RECODER.au32DMIC2PDMA_Buff[1];  //start address
		DMA_RXDESC[1].offset = (uint32_t)&DMA_RXDESC[0] - (PDMA->SCATBA);   //link to first description

		PDMA->DSCT[2].CTL = PDMA_OP_SCATTER;
		PDMA->DSCT[2].NEXT = (uint32_t)&DMA_RXDESC[0] - (PDMA->SCATBA);
		
		// Request source is memory to memory
		PDMA_SetTransferMode(2, PDMA_DMIC_RX, TRUE, (uint32_t)&DMA_RXDESC[1]);
		#endif 		
				
		// Enable PDMA channel 2 interrupt
		PDMA_EnableInt(2, PDMA_INT_TRANS_DONE);
	}
	
	NVIC_EnableIRQ(PDMA_IRQn);
}

//----------------------------------------------------------------------------
//  DPWM Initial function
//----------------------------------------------------------------------------
void DPWM_Init(uint32_t u32SampleRate)
{
	volatile uint32_t n;

#ifdef DPWM_GROUP_1
	SYS->GPC_MFPH &= ~(SYS_GPC_MFPH_PC10MFP_Msk | SYS_GPC_MFPH_PC11MFP_Msk | SYS_GPC_MFPH_PC12MFP_Msk | SYS_GPC_MFPH_PC13MFP_Msk);
	SYS->GPC_MFPH |= SYS_GPC_MFPH_PC10MFP_DPWM_RN |SYS_GPC_MFPH_PC11MFP_DPWM_RP | SYS_GPC_MFPH_PC12MFP_DPWM_LN | SYS_GPC_MFPH_PC13MFP_DPWM_LP;
#endif	
	
	// Select DPWM clock
	CLK_SetModuleClock(DPWM_MODULE, CLK_CLKSEL2_DPWMSEL_PLL, NULL);
	
	// Enable DPWM clock
	CLK_EnableModuleClock(DPWM_MODULE);
	
	// DPWM IPReset
	SYS_ResetModule(DPWM_RST);

	// DPWM control
	DPWM_SET_CLKSET(DPWM, DPWM_CLKSET_500FS);
	DPWM_SetSampleRate(u32SampleRate); //Set sample rate
	
	DPWM_ENABLE_FIFOTHRESHOLDINT(DPWM, 8);
	DPWM_SET_FIFODATAWIDTH(DPWM, DPWM_FIFO_DATAWIDTH_16BITS);
	DPWM_DISABLE_BIQUAD(DPWM);

	DPWM_ENABLE_PDMA(DPWM);

	// Start DPWM */
	//DPWM_START_PLAY(DPWM);
	DPWM_ENABLE_DRIVER(DPWM);
}
//----------------------------------------------------------------------------
//  UART0 Initial function
//----------------------------------------------------------------------------
void UART0_Init()
{
	// Enable UART module clock
	CLK_EnableModuleClock(UART0_MODULE);

	// Select UART clock source is HIRC
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

	// Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8)
	SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk);
	SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD);

	// Reset UART module
	SYS_ResetModule(UART0_RST);

	// Configure UART0 and set UART0 baud rate
	UART_Open(UART0, 115200);
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
