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
#include "audio_class.h"
#include "ConfigSysClk.h"

//----------------------------------------------------------------------------
//  Constant Define
//----------------------------------------------------------------------------
#define PDMA_ENABLE	(1)

//----------------------------------------------------------------------------
//  Playback Buffer Control Variable
//----------------------------------------------------------------------------
// Extern Variable Declaration
extern volatile uint32_t g_au32USB2PDMA_Buff[MAX_USB_BUFFER_LEN];
extern volatile uint32_t g_au32PDMA2I2S_Buff[2][PDMA2I2S_BUFF_LEN];
extern volatile uint8_t g_u8I2S_Buff_Index;
extern volatile uint8_t g_u8Update_USB_Buffer_Flag;
extern volatile uint16_t g_u16PlayBack_Read_Ptr;
extern volatile uint16_t g_u16PlayBack_Write_Ptr;

//----------------------------------------------------------------------------
//  Record Buffer Control Variable
//----------------------------------------------------------------------------
// Extern Variable Declaration
extern volatile uint8_t g_usbd_PlayMute;

typedef struct dma_desc_t 
{
    uint32_t ctl;
    uint32_t endsrc;
    uint32_t enddest;
    uint32_t offset;
} DMA_DESC_T;
DMA_DESC_T DMA_TXDESC[2], DMA_RXDESC[2];
//----------------------------------------------------------------------------
//  Functions Definition
//----------------------------------------------------------------------------
void USBD_Init(void);
void UART0_Init(void);
void PDMA_Init(void);
void HIRC_AutoTrim_Init(void);
void HIRC_AutoTrim_RefSof(void);
void I2S_Init(void);
void I2S_Master_Start(void);
void I2S_Master_Stop(void);
//----------------------------------------------------------------------------
//  MAIN function
//----------------------------------------------------------------------------
int main(void)
{	
	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();
	
	UART0_Init();
	
	HIRC_AutoTrim_Init();
	
	USBD_Init();
	USBD_Start();

	while(1)
	{
	}
}

//----------------------------------------------------------------------------
//  PDMA IRQ Handler function
//----------------------------------------------------------------------------
void PDMA_IRQHandler(void)
{
	int i;
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
		// channel 1 done 
		if(u32Transferdone & BIT1)
		{
			for ( i = 0 ; i < PDMA2I2S_BUFF_LEN ; i++)
			{
				if(g_u16PlayBack_Read_Ptr != g_u16PlayBack_Write_Ptr)
				{
					if(++g_u16PlayBack_Read_Ptr == MAX_USB_BUFFER_LEN)
						g_u16PlayBack_Read_Ptr = 0;
				
					if (g_usbd_PlayMute)
					{
						g_au32PDMA2I2S_Buff[g_u8I2S_Buff_Index][i] = 0x0;
					}
					else
					{
						g_au32PDMA2I2S_Buff[g_u8I2S_Buff_Index][i] = g_au32USB2PDMA_Buff[g_u16PlayBack_Read_Ptr];
					}
				}
				else
					g_au32PDMA2I2S_Buff[g_u8I2S_Buff_Index][i] = 0x0;
			}
			
			// switch to another buffer row.
			g_u8I2S_Buff_Index ^= 0x1;
		}
	}
}
//----------------------------------------------------------------------------
//  I2S IRQ Handler function
//----------------------------------------------------------------------------
void I2S0_IRQHandler() 
{
	UINT32 u32Data, u32Count;
	
	if(I2S_GET_INT_FLAG(I2S0, I2S_TXTH_INT_FLAG))
	{
		// Max write data count per time.
		u32Count = 8;
		while( !I2S_GET_TX_IS_FULL(I2S0) && u32Count != 0 )
		{
			if(g_u16PlayBack_Read_Ptr != g_u16PlayBack_Write_Ptr)
			{
				if(++g_u16PlayBack_Read_Ptr == MAX_USB_BUFFER_LEN)
					g_u16PlayBack_Read_Ptr = 0;
				
				u32Data = g_au32USB2PDMA_Buff[g_u16PlayBack_Read_Ptr];
				I2S_WRITE_TX_FIFO(I2S0,u32Data);
			}
			else
				I2S_WRITE_TX_FIFO(I2S0,0x00);
			
			u32Count--;
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
//  UART0 Initial function
//----------------------------------------------------------------------------
void USBD_Init(void)
{
	// MFP select 
	SYS->GPB_MFPH &= ~( SYS_GPB_MFPH_PB13MFP_Msk | SYS_GPB_MFPH_PB14MFP_Msk | SYS_GPB_MFPH_PB15MFP_Msk );
	SYS->GPB_MFPH |= SYS_GPB_MFPH_PB13MFP_USBD_DN | SYS_GPB_MFPH_PB14MFP_USBD_DP | SYS_GPB_MFPH_PB15MFP_USBD_VBUS;
	
	// Select IP clock source
	CLK_SetModuleClock(USBD_MODULE, CLK_CLKSEL4_USBSEL_HIRC, CLK_CLKDIV0_USBD(1));
	CLK_EnableModuleClock(USBD_MODULE);
	
	// USBD initial.
	USBD_Open(&gsInfo, AUDIO_ClassRequest, (SET_INTERFACE_REQ)AUDIO_SetInterface);
	
	// Endpoint configuration.
	AUDIO_Init();
	NVIC_EnableIRQ(USBD_IRQn);
}

//----------------------------------------------------------------------------
//  PDMA Initial function
//----------------------------------------------------------------------------
void PDMA_Init(void)
{
	// Enable PDMA engine clock.
	CLK_EnableModuleClock(PDMA_MODULE);
	// Reset PDMA module.
	SYS_ResetModule(PDMA_RST);
	
	PDMA_Open(PDMA_CH1_MASK);
	
	// USB_EP3 to PDMA to I2S.
	// Tx description.
	DMA_TXDESC[0].ctl = (((PDMA2I2S_BUFF_LEN)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		// transfer byte count   
												PDMA_WIDTH_32																		|		
												PDMA_SAR_INC																		|		// Source Address Increment
												PDMA_DAR_FIX																		|		// Destination Address fixed
												PDMA_REQ_SINGLE																	|		// Transfer Type = single
												PDMA_OP_SCATTER;																	// Basic mode
	DMA_TXDESC[0].endsrc = (uint32_t)&g_au32PDMA2I2S_Buff[0][0];    
	DMA_TXDESC[0].enddest = (uint32_t)&I2S0->TXFIFO;
	DMA_TXDESC[0].offset = ((uint32_t)&DMA_TXDESC[1] - (PDMA->SCATBA));

	DMA_TXDESC[1].ctl = (((PDMA2I2S_BUFF_LEN)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		// transfer byte count   
												PDMA_WIDTH_32																		|		
												PDMA_SAR_INC																	|		// Source Address Increment
												PDMA_DAR_FIX																	|		// Destination Address fixed
												PDMA_REQ_SINGLE																|		// Transfer Type = single
												PDMA_OP_SCATTER;																	// Basic mode											
	DMA_TXDESC[1].endsrc = (uint32_t)&g_au32PDMA2I2S_Buff[1][0];    
	DMA_TXDESC[1].enddest = (uint32_t)&I2S0->TXFIFO;
	DMA_TXDESC[1].offset = ((uint32_t)&DMA_TXDESC[0] - (PDMA->SCATBA));

	// Request source is memory to memory
	PDMA_SetTransferMode(1, PDMA_I2S0_TX, TRUE, DMA_TXDESC[1].offset);
	
	// Enable PDMA channel 1 interrupt
	PDMA_EnableInt(1, PDMA_INT_TRANS_DONE);

	NVIC_EnableIRQ(PDMA_IRQn);
}
//----------------------------------------------------------------------------
//  I2S Initial function
//----------------------------------------------------------------------------
#define I2S_MASTER_PIN_MASK     (SYS_GPD_MFPL_PD2MFP_Msk|SYS_GPD_MFPL_PD3MFP_Msk|SYS_GPD_MFPL_PD4MFP_Msk|SYS_GPD_MFPL_PD5MFP_Msk|SYS_GPD_MFPL_PD6MFP_Msk)
#define I2S_MASTER_PIN          (SYS_GPD_MFPL_PD2MFP_I2S0_MCLK|SYS_GPD_MFPL_PD3MFP_I2S0_LRCK|SYS_GPD_MFPL_PD4MFP_I2S0_DI|SYS_GPD_MFPL_PD5MFP_I2S0_DO|SYS_GPD_MFPL_PD6MFP_I2S0_BCLK)
#define I2S_MASTER_SAMPLE_RATE  (PLAY_RATE)
void I2S_Init(void)
{
	// Set I2S MFP
	SYS->GPD_MFPL = (SYS->GPD_MFPL&~I2S_MASTER_PIN_MASK)|I2S_MASTER_PIN;
	// Enable I2S0 clock.
	CLK_EnableModuleClock(I2S0_MODULE);
	// Select I2S0 clock.
	CLK_SetModuleClock(I2S0_MODULE, CLK_CLKSEL3_I2S0SEL_PLL, NULL);
	// I2S IPReset.
	SYS_ResetModule(I2S0_RST);
	
	// Open I2S0 hardware IP
	I2S_Open(I2S0, I2S_MASTER, 48000, I2S_DATABIT_16, I2S_TDMCHNUM_2CH, I2S_STEREO, I2S_FORMAT_I2S);
	// I2S0 Configuration
	I2S_SET_PCMSYNC(I2S0, I2S_PCMSYNC_BCLK);
	I2S_SET_MONO_RX_CHANNEL(I2S0, I2S_MONO_RX_RIGHT);
	I2S_SET_STEREOORDER(I2S0, I2S_ORDER_EVENLOW);
	// Set channel width.
	I2S_SET_CHWIDTH(I2S0, I2S_CHWIDTH_16);
	
#if (PDMA_ENABLE)
	// Enable Tx PDMA.	
	I2S_ENABLE_TXDMA(I2S0);
#else
	// Set FIFO threshold.
	I2S_SET_TXTH(I2S0, I2S_FIFO_TX_LEVEL_WORD_8);	
	// Enable interrupt.
	I2S_ENABLE_INT(I2S0, I2S_TXTH_INT_MASK);
	// Enable I2S's NVIC. 
	NVIC_EnableIRQ(I2S0_IRQn);
#endif
}
void I2S_Master_Start(void)
{
	// Clear TX FIFO buffer 
	I2S_CLR_TX_FIFO(I2S0);
	// Enable Tx function.
	I2S_ENABLE_TX(I2S0);
	// Enable I2S.
	I2S_ENABLE(I2S0);
}
void I2S_Master_Stop(void)
{
	I2S_DISABLE(I2S0);
	I2S_DISABLE_TX(I2S0);
	I2S_DISABLE_TXDMA(I2S0);
}

//----------------------------------------------------------------------------
//  UART0 Initial function
//----------------------------------------------------------------------------
void UART0_Init()
{
	// Enable UART module clock
	CLK_EnableModuleClock(UART0_MODULE);

	// Select UART clock source is HXT
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));
	//CLK->CLKSEL1 = (CLK->CLKSEL1 & ~CLK_CLKSEL1_UART0SEL_Msk) | (0x0 << CLK_CLKSEL1_UART0SEL_Pos);

	// Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8)
	SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk);
	SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD);

	// Reset UART module
	SYS_ResetModule(UART0_RST);

	// Configure UART0 and set UART0 baud rate
	UART_Open(UART0, 115200);

	printf("\nUART BR = 115200\n");
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
