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
#define DPWM_GROUP_1
#define DPWM_PDMA_16BIT

//----------------------------------------------------------------------------
//  Playback Buffer Control Variable
//----------------------------------------------------------------------------
// Extern Variable Declaration
extern volatile uint32_t g_au32USB2PDMA_Buff[MAX_USB_BUFFER_LEN];
extern volatile uint32_t g_au32PDMA2DPWM_Buff[2][MAX_USB_BUFFER_LEN];
extern volatile uint8_t g_u8DPWM_Buff_Index;

extern volatile uint16_t g_u16PlayBack_Read_Ptr;
extern volatile uint16_t g_u16PlayBack_Write_Ptr;
// Global Variable Declaration
volatile uint16_t g_u16PDMA2DPWM_Bufflen = 96;

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
void DPWM_Init(uint32_t u32SampleRate);
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
		
	// PDMA, DMIC initial.
	PDMA_Init();
	
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
			for ( i = 0 ; i < g_u16PDMA2DPWM_Bufflen ; i++)
			{
				if(g_u16PlayBack_Read_Ptr == g_u16PlayBack_Write_Ptr)
				{
					if(g_u16PlayBack_Read_Ptr != 0)
						g_u16PlayBack_Read_Ptr--;
					else
						g_u16PlayBack_Read_Ptr = MAX_USB_BUFFER_LEN-1;
				}
                if (g_usbd_PlayMute)
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
				{
					g_u16PlayBack_Read_Ptr = 0;
				}
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
//  UART0 Initial function
//----------------------------------------------------------------------------
void USBD_Init(void)
{
	// MFP select 
	SYS->GPB_MFPH &= ~( SYS_GPB_MFPH_PB13MFP_Msk | SYS_GPB_MFPH_PB14MFP_Msk | SYS_GPB_MFPH_PB15MFP_Msk );
	SYS->GPB_MFPH |= SYS_GPB_MFPH_PB13MFP_USBD_DN | SYS_GPB_MFPH_PB14MFP_USBD_DP | SYS_GPB_MFPH_PB15MFP_USBD_VBUS;
	
	// Select IP clock source
	CLK_SetModuleClock(USBD_MODULE, CLK_CLKSEL4_USBSEL_PLL, CLK_CLKDIV0_USBD(4));
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
	
	// USB_EP3 to PDMA to DPWM.
#ifdef DPWM_PDMA_32BIT
	// Tx description.
	DMA_TXDESC[0].ctl = (((PDMA2DPWM_BUFF_LEN)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		// transfer byte count   
													PDMA_WIDTH_32																		|		// 32 bit 
													PDMA_SAR_INC																		|		// Source Address Increment
													PDMA_DAR_FIX																		|		// Destination Address fixed
													PDMA_REQ_SINGLE																	|		// Transfer Type = single
													PDMA_OP_SCATTER;																	// Basic mode
	DMA_TXDESC[0].endsrc = (uint32_t)&g_au32PDMA2DPWM_Buff[0][0];    
	DMA_TXDESC[0].enddest = (uint32_t)&DPWM->FIFO;
	DMA_TXDESC[0].offset = ((uint32_t)&DMA_TXDESC[1] - (PDMA->SCATBA));

	DMA_TXDESC[1].ctl = (((PDMA2DPWM_BUFF_LEN)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		// transfer byte count   
												PDMA_WIDTH_32																		|		// 32 bit 
												PDMA_SAR_INC																	|		// Source Address Increment
												PDMA_DAR_FIX																	|		// Destination Address fixed
												PDMA_REQ_SINGLE																|		// Transfer Type = single
												PDMA_OP_SCATTER;																	// Basic mode											
	DMA_TXDESC[1].endsrc = (uint32_t)&g_au32PDMA2DPWM_Buff[1][0];    
	DMA_TXDESC[1].enddest = (uint32_t)&DPWM->FIFO;
	DMA_TXDESC[1].offset = ((uint32_t)&DMA_TXDESC[0] - (PDMA->SCATBA));

	// Request source is memory to memory
	PDMA_SetTransferMode(1, PDMA_DPWM_TX, TRUE, DMA_TXDESC[1].offset);
#endif 


#ifdef DPWM_PDMA_16BIT
	// Tx description.
	DMA_TXDESC[0].ctl = (((PDMA2DPWM_BUFF_LEN)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		// transfer byte count   
													PDMA_WIDTH_16																		|		
													PDMA_SAR_INC																		|		// Source Address Increment
													PDMA_DAR_FIX																		|		// Destination Address fixed
													PDMA_REQ_SINGLE																	|		// Transfer Type = single
													PDMA_OP_SCATTER;																	// Basic mode
	DMA_TXDESC[0].endsrc = (uint32_t)&g_au32PDMA2DPWM_Buff[0][0];    
	DMA_TXDESC[0].enddest = (uint32_t)&DPWM->FIFO;
	DMA_TXDESC[0].offset = ((uint32_t)&DMA_TXDESC[1] - (PDMA->SCATBA));

	DMA_TXDESC[1].ctl = (((PDMA2DPWM_BUFF_LEN)-1)<<PDMA_DSCT_CTL_TXCNT_Pos)	|		// transfer byte count   
												PDMA_WIDTH_16																		|		
												PDMA_SAR_INC																	|		// Source Address Increment
												PDMA_DAR_FIX																	|		// Destination Address fixed
												PDMA_REQ_SINGLE																|		// Transfer Type = single
												PDMA_OP_SCATTER;																	// Basic mode											
	DMA_TXDESC[1].endsrc = (uint32_t)&g_au32PDMA2DPWM_Buff[1][0];    
	DMA_TXDESC[1].enddest = (uint32_t)&DPWM->FIFO;
	DMA_TXDESC[1].offset = ((uint32_t)&DMA_TXDESC[0] - (PDMA->SCATBA));

	// Request source is memory to memory
	PDMA_SetTransferMode(1, PDMA_DPWM_TX, TRUE, DMA_TXDESC[1].offset);
#endif 
	
	// Enable PDMA channel 1 interrupt
	PDMA_EnableInt(1, PDMA_INT_TRANS_DONE);

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
	CLK_SetModuleClock(DPWM_MODULE, CLK_CLKSEL2_DPWMSEL_PCLK0, NULL);
	
	// Enable DPWM clock
	CLK_EnableModuleClock(DPWM_MODULE);
	
	// DPWM IPReset
	SYS_ResetModule(DPWM_RST);

	// DPWM control
	DPWM_SET_CLKSET(DPWM, DPWM_CLKSET_500FS);
	DPWM_SetSampleRate(u32SampleRate); //Set sample rate
	
	DPWM_ENABLE_FIFOTHRESHOLDINT(DPWM, 8);
//	DPWM_SET_FIFODATAWIDTH(DPWM, DPWM_FIFO_DATAWIDTH_24BITS);
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
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
