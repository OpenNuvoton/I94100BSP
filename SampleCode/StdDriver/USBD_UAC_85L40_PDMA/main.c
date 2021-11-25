/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 19/11/25 13:36 $
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
void MIC_Init(void);
void MIC_Start(void);
void MIC_Stop(void);
void HIRC_AutoTrim_Init(void);
void HIRC_AutoTrim_RefSof(void);
/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables and constants                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
#define AMIC_PDMA_16BIT
// 85L40 Buffer
uint32_t g_u32MICBuffer[2][AMIC2PDMA_BUFF_LEN] = {0};

/*---------------------------------------------------------------------------------------------------------*/
/*  MAIN function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int main(void)
{	
	/* Init System, peripheral clock and multi-function I/O */
	SYSCLK_INITIATE();

	// Enable PDMA clock. 
	CLK_EnableModuleClock(PDMA_MODULE);
	// Reset PDMA module
	SYS_ResetModule(PDMA_RST);
	// Enable PDMA's NVIC
	NVIC_EnableIRQ(PDMA_IRQn);
	
	// Initiate microphone.
	MIC_Init();
	// Stop microphone. Wait for UAC to start Microphone.
	MIC_Stop();

	HIRC_AutoTrim_Init();
	
	// Initiate UAC to playback audio on PC.
	UAC_Init();
	// Start UAC. 
	UAC_Start();
	
	while(1);
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
				SYS_EnableTrimHIRC(SYS_IRCTCTL_FREQSEL_49M);
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
	SYS_EnableTrimHIRC(SYS_IRCTCTL_FREQSEL_49M);

	while(!SYS_IS_TRIMHIRC_DONE())
	{
	}
	
	SYS_CLEAR_TRIMHIRC_INT_FLAG(SYS_IRCTISTS_TRIMFAIL_INT_FLAG|SYS_IRCTISTS_CLKERROR_INT_FLAG);
}

// Microphone(85L40) ============================================================================================
#define MIC_I2C1_PIN_MASK  (SYS_GPD_MFPH_PD14MFP_Msk|SYS_GPD_MFPH_PD15MFP_Msk)
#define MIC_I2C1_PIN       (SYS_GPD_MFPH_PD14MFP_I2C1_SCL|SYS_GPD_MFPH_PD15MFP_I2C1_SDA)
#define MIC_I2C1_BUS_FREQ  (100000)   
#define MIC_I2C1_MCLK_FREQ (4096000)
#define MIC_I2C1_DEV_ADDR  (0x1C)

#define MIC_PDMA_CH        (3)

#define MIC_I2S0_PIN_MASK  (SYS_GPD_MFPL_PD2MFP_Msk|SYS_GPD_MFPL_PD3MFP_Msk|SYS_GPD_MFPL_PD4MFP_Msk|SYS_GPD_MFPL_PD5MFP_Msk|SYS_GPD_MFPL_PD6MFP_Msk)
#define MIC_I2S0_PIN       (SYS_GPD_MFPL_PD2MFP_I2S0_MCLK|SYS_GPD_MFPL_PD3MFP_I2S0_LRCK|SYS_GPD_MFPL_PD4MFP_I2S0_DI|SYS_GPD_MFPL_PD5MFP_I2S0_DO|SYS_GPD_MFPL_PD6MFP_I2S0_BCLK)

typedef struct{
	uint8_t  u8DeviceAddr;
	uint16_t u16Counter;
	uint16_t u16MaxCount;
	uint8_t* pau8Cmd;
} S_MIC_I2CCTRL;

typedef struct{
	uint8_t  u8Reg[2];
	uint8_t  u8Value[2];
} S_MIC_I2CCMD;

DSCT_T sPDMA_MIC[2];
volatile S_MIC_I2CCTRL s_MIC_I2CCtrl;      	// Provide microphone send command to 85L40S_BUFCTRL* psSPK_BufCtrl = NULL;

// Command for 85L40(transfer via I2C1)
S_MIC_I2CCMD const asMIC_Cmd_85L40[] = {
//-------
{	0x00	,	0x00	,	0x00	,	0x01	}	,
//-------
{	0x00	,	0x03	,	0x00	,	0x40	}	,
{	0x00	,	0x04	,	0x00	,	0x01	}	,
{	0x00	,	0x05	,	0x31	,	0x26	}	,
{	0x00	,	0x06	,	0x00	,	0x08	}	,
{	0x00	,	0x07	,	0x00	,	0x10	}	,
{	0x00	,	0x08	,	0xC0	,	0x00	}	,
{	0x00	,	0x09	,	0xE0	,	0x00	}	,
{	0x00	,	0x0A	,	0xF1	,	0x3C	}	,
#if (AMIC_BIT_RES == 32) 
{	0x00	,	0x10	,	0x00	,	0x4F	}	,  //PCMB, 32Bit
#endif 
#if (AMIC_BIT_RES == 16)
{	0x00	,	0x10	,	0x00	,	0x43	}	,  	//PCMB, 16Bit
#endif 
{	0x00	,	0x11	,	0x00	,	0x00	}	,
{	0x00	,	0x12	,	0x00	,	0x00	}	,
{	0x00	,	0x13	,	0x00	,	0x00	}	,
#if (UAC_REC_CHANNEL == 1)
{	0x00	,	0x14	,	0xC0	,	0x01	}	,		
#endif
#if (UAC_REC_CHANNEL == 2)	
{	0x00	,	0x14	,	0xC0	,	0x03	}	,		
#endif
#if (UAC_REC_CHANNEL == 4)	
{	0x00	,	0x14	,	0xC0	,	0x0F	}	,		
#endif
{	0x00	,	0x20	,	0x00	,	0x00	}	,
{	0x00	,	0x21	,	0x70	,	0x0B	}	,
{	0x00	,	0x22	,	0x00	,	0x22	}	,
{	0x00	,	0x23	,	0x10	,	0x10	}	,
{	0x00	,	0x24	,	0x10	,	0x10	}	,
{	0x00	,	0x2D	,	0x10	,	0x10	}	,
{	0x00	,	0x2E	,	0x10	,	0x10	}	,
{	0x00	,	0x2F	,	0x00	,	0x00	}	,
{	0x00	,	0x30	,	0x00	,	0x00	}	,
{	0x00	,	0x31	,	0x00	,	0x00	}	,
{	0x00	,	0x32	,	0x00	,	0x00	}	,
{	0x00	,	0x33	,	0x00	,	0x00	}	,
{	0x00	,	0x34	,	0x00	,	0x00	}	,
{	0x00	,	0x35	,	0x00	,	0x00	}	,
{	0x00	,	0x36	,	0x00	,	0x00	}	,
{	0x00	,	0x37	,	0x00	,	0x00	}	,
{	0x00	,	0x38	,	0x00	,	0x00	}	,
{	0x00	,	0x39	,	0x00	,	0x00	}	,
{	0x00	,	0x3A	,	0x40	,	0x62	}	,		//16K SR
//{	0x00	,	0x40	,	0x04	,	0x0A	}	,
//{	0x00	,	0x41	,	0x04	,	0x0A	}	,
//{	0x00	,	0x42	,	0x04	,	0x0A	}	,
//{	0x00	,	0x43	,	0x04	,	0x0A	}	,
{	0x00	,	0x40	,	0x04	,	0x08	}	,  //DGAIN = 0dB
{	0x00	,	0x41	,	0x04	,	0x08	}	,  //DGAIN = 0dB
{	0x00	,	0x42	,	0x04	,	0x08	}	,  //DGAIN = 0dB
{	0x00	,	0x43	,	0x04	,	0x08	}	,  //DGAIN = 0dB
{	0x00	,	0x44	,	0x00	,	0xE4	}	,
{	0x00	,	0x48	,	0x00	,	0x00	}	,
{	0x00	,	0x49	,	0x00	,	0x00	}	,
{	0x00	,	0x4A	,	0x00	,	0x00	}	,
{	0x00	,	0x4B	,	0x00	,	0x00	}	,
{	0x00	,	0x4C	,	0x00	,	0x00	}	,
{	0x00	,	0x4D	,	0x00	,	0x00	}	,
{	0x00	,	0x4E	,	0x00	,	0x00	}	,
{	0x00	,	0x4F	,	0x00	,	0x00	}	,
{	0x00	,	0x50	,	0x00	,	0x00	}	,
{	0x00	,	0x51	,	0x00	,	0x00	}	,
{	0x00	,	0x52	,	0xEF	,	0xFF	}	,
{	0x00	,	0x57	,	0x00	,	0x00	}	,
{	0x00	,	0x58	,	0x1C	,	0xF0	}	,
{	0x00	,	0x59	,	0x00	,	0x08	}	,
{	0x00	,	0x60	,	0x00	,	0x60	}	,		// VMID_CTRL 
{	0x00	,	0x61	,	0x00	,	0x00	}	,
{	0x00	,	0x62	,	0x00	,	0x00	}	,
{	0x00	,	0x63	,	0x00	,	0x00	}	,
{	0x00	,	0x64	,	0x00	,	0x11	}	,
{	0x00	,	0x65	,	0x02	,	0x20	}	,
{	0x00	,	0x66	,	0x00	,	0x0F	}	,
{	0x00	,	0x67	,	0x0D	,	0x04	}	,
{	0x00	,	0x68	,	0x70	,	0x00	}	,
{	0x00	,	0x69	,	0x00	,	0x00	}	,
{	0x00	,	0x6A	,	0x00	,	0x00	}	,
//{	0x00	,	0x6B	,	0x1B	,	0x1B	}	,  //AGAIN = 26dB
//{	0x00	,	0x6C	,	0x1B	,	0x1B	}	,  //AGAIN = 26dB
{	0x00	,	0x6B	,	0x20	,	0x20	}	,  //AGAIN = 32dB
{	0x00	,	0x6C	,	0x20	,	0x20	}	,  //AGAIN = 32dB
{	0x00	,	0x6D	,	0xF0	,	0x00	}	,
{	0x00	,	0x01	,	0x00	,	0x0F	}	,
{	0x00	,	0x02	,	0x80	,	0x03	}	
};

void MIC_Init(void)
{
	// (1) Configure I2C1 for sending command to 85L40
	// (1-1) Initiate I2C1
	{
		// Reset module. 
		SYS_ResetModule(I2C1_RST);
		// Enable I2C0 module clock. 
		CLK_EnableModuleClock(I2C1_MODULE);
		// Open I2C module and set bus clock. 
		I2C_Open(I2C1, MIC_I2C1_BUS_FREQ);
		// Enable I2C interrupt. 
		I2C_EnableInt(I2C1);
		NVIC_EnableIRQ(I2C1_IRQn);	
		// GPIO multi-function. (GPD14:I2C1_SCL, GPD15:I2C1_SDA) 
		SYS->GPD_MFPH = (SYS->GPD_MFPH & ~MIC_I2C1_PIN_MASK)|MIC_I2C1_PIN;
	}		
	// (1-2) Send command to 85L40 via I2C1
	{
		uint16_t u16i;
		I2C_SetBusClockFreq(I2C1,MIC_I2C1_BUS_FREQ);
		s_MIC_I2CCtrl.u8DeviceAddr = MIC_I2C1_DEV_ADDR;
		for(u16i=0;u16i<sizeof(asMIC_Cmd_85L40)/sizeof(S_MIC_I2CCMD);u16i++) 
		{
			s_MIC_I2CCtrl.pau8Cmd = (uint8_t*)&asMIC_Cmd_85L40[u16i];
			s_MIC_I2CCtrl.u16Counter = 0;
			s_MIC_I2CCtrl.u16MaxCount = sizeof(S_MIC_I2CCMD);
			I2C_START(I2C1);
			/* Wait for I2C transmit completed. */
			while(s_MIC_I2CCtrl.u16MaxCount>0);
		}		
	}
	
	// (2) Configure I2S to ge voice data from 85L40
	{		
		// PD2: I2S0_MCLK, PD3: I2S0_LRCK, PD4: I2S0_DI, PD5: I2S0_DO, PD6: I2S0_BCLK
		SYS->GPD_MFPL = (SYS->GPD_MFPL & ~MIC_I2S0_PIN_MASK)|MIC_I2S0_PIN;
		// Enable I2S clock. 
		CLK_EnableModuleClock(I2S0_MODULE);
		// Select I2S clock. 	
		CLK_SetModuleClock(I2S0_MODULE, CLK_CLKSEL3_I2S0SEL_PLL, NULL);		
		// I2S IPReset. 
		SYS_ResetModule(I2S0_RST);
		// Open I2S and enable master clock. 
#if (UAC_REC_CHANNEL == 1)	
		I2S_Open(I2S0, I2S_MASTER, AMIC_REC_RATE, I2S_DATABIT_16, I2S_TDMCHNUM_2CH, I2S_STEREO, I2S_FORMAT_PCMMSB);
#endif 	
#if (UAC_REC_CHANNEL == 2)	
		I2S_Open(I2S0, I2S_MASTER, AMIC_REC_RATE, I2S_DATABIT_16, I2S_TDMCHNUM_2CH, I2S_STEREO, I2S_FORMAT_PCMMSB);
#endif 	
#if (UAC_REC_CHANNEL == 4)	
		I2S_Open(I2S0, I2S_MASTER, AMIC_REC_RATE, I2S_DATABIT_16, I2S_TDMCHNUM_4CH, I2S_STEREO, I2S_FORMAT_PCMMSB);
#endif
		// Enable I2C0 MCLK
		I2S_EnableMCLK(I2S0, MIC_I2C1_MCLK_FREQ);		
		// I2S Configuration. 
		I2S_SET_PCMSYNC(I2S0, I2S_PCMSYNC_BCLK);
		// Set RX channel for Mono mode.
#if (UAC_REC_CHANNEL == 1)
		I2S_SET_MONO_RX_CHANNEL(I2S0, I2S_MONO_RX_LEFT);
#endif 
#if (UAC_REC_CHANNEL == 2)		
		I2S_SET_MONO_RX_CHANNEL(I2S0, I2S_MONO_RX_RIGHT);
#endif
		// Set data order.
		I2S_SET_STEREOORDER(I2S0, I2S_ORDER_EVENLOW);
		// Enable I2S RXDMA
		I2S_ENABLE_RXDMA(I2S0);
		// Set channel width. 
#if (AMIC_BIT_RES == 16) 
		I2S_SET_CHWIDTH(I2S0, I2S_CHWIDTH_16);
#endif 
#if (AMIC_BIT_RES == 32) 
		I2S_SET_CHWIDTH(I2S0, I2S_CHWIDTH_32);
#endif 
		// Set 16 bits data width.
		I2S_SET_PBWIDTH(I2S0, I2S_PBWIDTH_16);
		// Set FIFO Read/Write Order in 16-bit Width of Peripheral Bus.
		I2S_SET_PB16ORD(I2S0, I2S_PB16ORD_LOW);
		// Set FIFO threshold. 
		I2S_SET_RXTH(I2S0, I2S_FIFO_RX_LEVEL_WORD_9);
		// Clear TX, RX FIFO buffer 
		I2S_CLR_TX_FIFO(I2S0);
		I2S_CLR_RX_FIFO(I2S0);
	}
	// (3) Config PDMA for I2S transfer data.	
	{
#ifdef AMIC_PDMA_16BIT
	/* Rx description */
	sPDMA_MIC[0].CTL = ((AMIC2PDMA_BUFF_LEN-1)<<PDMA_DSCT_CTL_TXCNT_Pos)|
												PDMA_WIDTH_16		|
												PDMA_SAR_FIX		|
												PDMA_DAR_INC		|
												PDMA_REQ_SINGLE	|
												PDMA_OP_SCATTER;
	sPDMA_MIC[0].SA = (uint32_t)&I2S0->RXFIFO;
	sPDMA_MIC[0].DA = (uint32_t)&g_u32MICBuffer[0];
	sPDMA_MIC[0].NEXT = (uint32_t)&sPDMA_MIC[1] - (PDMA->SCATBA);

	sPDMA_MIC[1].CTL = ((AMIC2PDMA_BUFF_LEN-1)<<PDMA_DSCT_CTL_TXCNT_Pos)|
												PDMA_WIDTH_16		|
												PDMA_SAR_FIX		|
												PDMA_DAR_INC		|
												PDMA_REQ_SINGLE	|
												PDMA_OP_SCATTER;
	sPDMA_MIC[1].SA = (uint32_t)&I2S0->RXFIFO;
	sPDMA_MIC[1].DA = (uint32_t)&g_u32MICBuffer[1];  
	sPDMA_MIC[1].NEXT = (uint32_t)&sPDMA_MIC[0] - (PDMA->SCATBA);   //link to first description	
#endif 		 
	}
}

void MIC_Start(void)
{
	// Open PDMA channel
	PDMA_Open((1<<MIC_PDMA_CH));
	// Set TransMode
	PDMA_SetTransferMode(MIC_PDMA_CH, PDMA_I2S0_RX, TRUE, (uint32_t)&sPDMA_MIC[0]);
	// Enable interrupt
	PDMA_EnableInt(MIC_PDMA_CH,PDMA_INT_TRANS_DONE);

	I2S_ENABLE(I2S0);
	I2S_ENABLE_RX(I2S0);	
}

void MIC_Stop(void)
{
		I2S_DISABLE_RX(I2S0);
		I2S_DISABLE(I2S0);
}

void I2C1_IRQHandler() 
{
	if(I2C_GET_TIMEOUT_FLAG(I2C1)) 
		I2C_ClearTimeoutFlag(I2C1); 
	else 
	{
		switch(I2C_GET_STATUS(I2C1)) 
		{
			// START has been transmitted and Write SLA+W to Register I2CDAT.
			case 0x08:
				I2C_SET_DATA(I2C1, s_MIC_I2CCtrl.u8DeviceAddr << 1);    
				I2C_SET_CONTROL_REG(I2C1, I2C_CTL_SI);			
			break;
			// SLA+W has been transmitted and ACK has been received.
			case 0x18:
				I2C_SET_DATA(I2C1, s_MIC_I2CCtrl.pau8Cmd[s_MIC_I2CCtrl.u16Counter++]);
				I2C_SET_CONTROL_REG(I2C1, I2C_CTL_SI);		
			break;
			// SLA+W has been transmitted and NACK has been received.
			case 0x20:
				I2C_STOP(I2C1);
				I2C_START(I2C1);	
				s_MIC_I2CCtrl.u16MaxCount = 0;
			break;
			// DATA has been transmitted and ACK has been received.
			case 0x28:
				if(s_MIC_I2CCtrl.u16Counter < s_MIC_I2CCtrl.u16MaxCount) 
				{
					I2C_SET_DATA(I2C1, s_MIC_I2CCtrl.pau8Cmd[s_MIC_I2CCtrl.u16Counter++]);
					I2C_SET_CONTROL_REG(I2C1, I2C_CTL_SI);
				} 
				else 
				{
					I2C_SET_CONTROL_REG(I2C1, I2C_CTL_STO_SI);
					// transfer complete
					s_MIC_I2CCtrl.u16MaxCount = 0;		
				}
			break;
		}
	}
}

// PMDA =========================================================================================================
/* PDMA TDF Bit Field Definitions */
#define PDMA_TDF_TD_F_Pos           PDMA_TDSTS_TDIF0_Pos                         /*!< PDMA TDF: TD_Fx Position */
#define PDMA_TDF_TD_F_Msk           (0xFFFFul << PDMA_TDF_TD_F_Pos)              /*!< PDMA TDF: TD_Fx Mask */
#define PDMA_ABTF_TABORT_F_Pos      PDMA_ABTSTS_ABTIF0_Pos                       /*!< PDMA ABTF: TABORT_Fx Position */
#define PDMA_ABTF_TABORT_F_Msk      (0xFFFFul << PDMA_ABTF_TABORT_F_Pos)         /*!< PDMA ABTF: TABORT_Fx Mask */

volatile uint32_t g_u32amic_pdma_bufidx = 1;
// Extern from Audio
extern volatile uint8_t g_usbd_UsbAudioState;
extern volatile uint32_t g_au32UAC_RingBuff[];		
extern volatile uint16_t g_u16UAC_Buff_ReadIndex;
extern volatile uint16_t g_u16UAC_Buff_WriteIndex;

void PDMA_IRQHandler(void)
{
	int i;
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
			if (g_usbd_UsbAudioState == UAC_START_AUDIO_RECORD)
			{		
				for ( i= 0; i < (AMIC2PDMA_BUFF_LEN/2); i++)
				{
					g_au32UAC_RingBuff[g_u16UAC_Buff_WriteIndex++] = g_u32MICBuffer[0][i];
				}	
				g_usbd_UsbAudioState = UAC_PROCESS_AUDIO_RECORD;
			}
			else if (g_usbd_UsbAudioState == UAC_PROCESS_AUDIO_RECORD)
			{
				for ( i= 0 ; i < (AMIC2PDMA_BUFF_LEN/2) ; i++)
				{
					g_au32UAC_RingBuff[g_u16UAC_Buff_WriteIndex++] = g_u32MICBuffer[1][i];
				}
				g_usbd_UsbAudioState = UAC_READY_AUDIO_RECORD;
			}
			else if (g_usbd_UsbAudioState == UAC_READY_AUDIO_RECORD)
			{
				for ( i= 0 ; i < (AMIC2PDMA_BUFF_LEN/2) ; i++)
				{
					g_au32UAC_RingBuff[g_u16UAC_Buff_WriteIndex++] = g_u32MICBuffer[0][i];
				}
				g_usbd_UsbAudioState = UAC_BUSY_AUDIO_RECORD;
				g_u32amic_pdma_bufidx = 1;
			}
			else if (g_usbd_UsbAudioState == UAC_BUSY_AUDIO_RECORD)
			{
				for ( i= 0 ; i < (AMIC2PDMA_BUFF_LEN/2) ; i++)
				{
					g_au32UAC_RingBuff[g_u16UAC_Buff_WriteIndex++] = g_u32MICBuffer[g_u32amic_pdma_bufidx][i];
				
					if ( g_u16UAC_Buff_WriteIndex == AMIC_RING_BUFFER_LEN ) 
						g_u16UAC_Buff_WriteIndex = 0;
				}
				g_u32amic_pdma_bufidx ^= 0x1;	
			}
		}
	}
}
/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
