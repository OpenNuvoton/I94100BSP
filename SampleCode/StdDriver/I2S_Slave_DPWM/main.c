/**************************************************************************//**
 * @file     main.c
 * @version  V0.10
 * $Revision: 11 $
 * $Date: 15/09/02 10:04a $
 * @brief
 *           Configure SPI1 as SPI_I2S Slave mode and demonstrate how SPI_I2S works in Slave mode.
 *           This sample code needs to work with SPI_I2S_Master.
 * @note
 * Copyright (C) 2014~2015 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "BUFCTRL.h"
#include "ConfigSysClk.h"

#define DEMO_DATA_COUNT (256)

#define DPWM_GROUP_1
#define DPWM_PDMA_16BIT

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void UART0_Initiate(void);
void I2S0_Slave_Initiate(S_BUFCTRL* psInBufCtrl,S_BUFCTRL* psOutBufCtrl);
void I2S0_Slave_Start(void);
void DPWM_Init(uint32_t u32SampleRate);

/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables and constants                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
volatile S_BUFCTRL sInBufCtrl,sOutBufCtrl; // Buffer control handler.
int32_t ai32InBuf[DEMO_DATA_COUNT];        // Buffer array: provide I2S_Salve receiver data. 
int32_t ai32OutBuf[DEMO_DATA_COUNT];       // Buffer array: provide I2S_Slave send data. 

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	// These defines are from  BUFCTRL.h for buffer control in this samle. 
	// Buffer control handler configuration. 
	BUFCTRL_CFG((&sInBufCtrl),ai32InBuf,sizeof(ai32InBuf)/sizeof(uint32_t));

	// Initiate UART0 for print message.
	UART0_Initiate();
	
	// Initiate I2S slave via I2S0
	I2S0_Slave_Initiate((S_BUFCTRL*)(&sInBufCtrl), NULL);//(S_BUFCTRL*)(&sOutBufCtrl));
	
	DPWM_Init(48000);

	printf("+----------------------------------------------------------+\n");
	printf("|            SPI_I2S Driver Sample Code (slave mode)           |\n");
	printf("+----------------------------------------------------------+\n");
	printf("  I2S0 configuration:\n");
	printf("      Sample rate 48 KHz\n");
	printf("      Word width 16 bits\n");
	printf("      Stereo mode\n");
	printf("      I2S format\n");
	printf("  The I/O connection for I2S0:\n");
	printf("  Master PD6(BCLK) < - > Slave PD6(BCLK)\n");
	printf("  Master PD5(DO) < - > Slave PD4(DI)\n");
	printf("  Master PD4(DI) < - > Slave PD5(DO)\n");
	printf("	Master PD3(LRCK) < - > Slave PD3(LRCK)\n");
	printf("  NOTE: Connect with a I2S master.\n");
	
	// Start send data from output buffer and receive data into input buffer.
	I2S0_Slave_Start();
	
	while(1)
	{
	}
}

// I2S_SLAVE ===================================================================================================
#define I2S_SLAVE_PIN_MASK     (SYS_GPD_MFPL_PD2MFP_Msk|SYS_GPD_MFPL_PD3MFP_Msk|SYS_GPD_MFPL_PD4MFP_Msk|SYS_GPD_MFPL_PD5MFP_Msk|SYS_GPD_MFPL_PD6MFP_Msk)
#define I2S_SLAVE_PIN          (SYS_GPD_MFPL_PD2MFP_I2S0_MCLK|SYS_GPD_MFPL_PD3MFP_I2S0_LRCK|SYS_GPD_MFPL_PD4MFP_I2S0_DI|SYS_GPD_MFPL_PD5MFP_I2S0_DO|SYS_GPD_MFPL_PD6MFP_I2S0_BCLK)

// Provide I2S0 slave's buffer control.
S_BUFCTRL* g_psI2S0_InBufCtrl = NULL;
S_BUFCTRL* g_psI2S0_OutBufCtrl = NULL;

void I2S0_Slave_Initiate(S_BUFCTRL* psInBufCtrl,S_BUFCTRL* psOutBufCtrl)
{
	// Set I2S MFP
	SYS->GPD_MFPL = (SYS->GPD_MFPL&~I2S_SLAVE_PIN_MASK)|I2S_SLAVE_PIN;
	
	// Enable I2S0 clock.
	CLK_EnableModuleClock(I2S0_MODULE);
	// Select I2S0 clock.
	CLK_SetModuleClock(I2S0_MODULE, CLK_CLKSEL3_I2S0SEL_PLL, NULL);
	// I2S IPReset.
	SYS_ResetModule(I2S0_RST);	
	// Open I2S0 hardware IP
	I2S_Open(I2S0, I2S_SLAVE, 0, I2S_DATABIT_16, I2S_TDMCHNUM_2CH, I2S_STEREO, I2S_FORMAT_I2S);
	// I2S0 Configuration
	I2S_SET_PCMSYNC(I2S0, I2S_PCMSYNC_BCLK);
	I2S_SET_MONO_RX_CHANNEL(I2S0, I2S_MONO_RX_RIGHT);
	I2S_SET_STEREOORDER(I2S0, I2S_ORDER_EVENLOW);
	// Set channel width.
	I2S_SET_CHWIDTH(I2S0, I2S_CHWIDTH_16);
	// Set FIFO threshold.
	I2S_SET_TXTH(I2S0, I2S_FIFO_TX_LEVEL_WORD_8);
	I2S_SET_RXTH(I2S0, I2S_FIFO_RX_LEVEL_WORD_8);
	// Enable interrupt.
	I2S_ENABLE_INT(I2S0, I2S_RXTH_INT_MASK);
	// Enable I2S's NVIC. 
	NVIC_EnableIRQ(I2S0_IRQn);
	
	// Set buffer control pointer.
	if(psInBufCtrl != NULL)
		g_psI2S0_InBufCtrl = psInBufCtrl;
	
	if(psOutBufCtrl != NULL)
		g_psI2S0_OutBufCtrl = psOutBufCtrl;
}

void I2S0_Slave_Start(void)
{
	UINT32 u32Data;
	
	if( g_psI2S0_InBufCtrl != NULL )
	{
		// Clear TX, RX FIFO buffer 
		I2S_CLR_RX_FIFO(I2S0);
		// Enable Rx function
		I2S_ENABLE_RX(I2S0);	
	}
	if( g_psI2S0_OutBufCtrl != NULL )
	{
		// Clear TX FIFO buffer 
		I2S_CLR_TX_FIFO(I2S0);
		// Set send data into TX FIFO buffer;
		while( !I2S_GET_TX_IS_FULL(I2S0) && !BUFCTRL_IS_EMPTY(g_psI2S0_OutBufCtrl) )
		{
			BUFCTRL_READ(g_psI2S0_OutBufCtrl,&u32Data);
			I2S_WRITE_TX_FIFO(I2S0,u32Data);
		}
		// Enable Tx function.
		I2S_ENABLE_TX(I2S0);
	}			
	if( g_psI2S0_InBufCtrl != NULL || g_psI2S0_OutBufCtrl != NULL )
	{
		I2S_ENABLE(I2S0);	
	}
}

void I2S0_IRQHandler() 
{
	UINT32 u32Data, u32Count;
	
	if(I2S_GET_INT_FLAG(I2S0, I2S_TXTH_INT_FLAG))
	{
		// Write data process.
		if( g_psI2S0_OutBufCtrl != NULL )
		{
			// Max write data count per time.
			u32Count = 8; 		
			while( !I2S_GET_TX_IS_FULL(I2S0) && u32Count != 0 )
			{
				if(!BUFCTRL_IS_EMPTY(g_psI2S0_OutBufCtrl))
				{
					BUFCTRL_READ(g_psI2S0_OutBufCtrl,&u32Data);
					I2S_WRITE_TX_FIFO(I2S0,u32Data);
				}
				else
					I2S_WRITE_TX_FIFO(I2S0,0x00);
				
				u32Count--;
			}
		}
	}
	
	if(I2S_GET_INT_FLAG(I2S0, I2S_RXTH_INT_FLAG))
	{
		// Read data process.
		if( g_psI2S0_InBufCtrl != NULL )
		{
			// Max read data count per time.
			u32Count = 8; 
			while( !I2S_GET_RX_IS_EMPTY(I2S0) && u32Count != 0 )
			{
				// Read the data from I2S RXFIFO.
				u32Data = I2S_READ_RX_FIFO(I2S0);
				if( !BUFCTRL_IS_FULL(g_psI2S0_InBufCtrl) )
					BUFCTRL_WRITE(g_psI2S0_InBufCtrl,u32Data);
				u32Count--;	
			}
		}
	}
}

//----------------------------------------------------------------------------
//  DPWM Initial function
//----------------------------------------------------------------------------
void DPWM_IRQHandler()
{
	uint32_t u32Temp;
	uint8_t u8Count = 0;
	
	while(!DPWM_IS_FIFOFULL(DPWM) && u8Count < 8)
	{
		if(!BUFCTRL_IS_EMPTY((&sInBufCtrl)))
		{
			// Check if FIFO has enough space to put L&R channels data.
			if(DPWM_GET_FIFOPOINTER(DPWM) >= 29)
				return;
			
			BUFCTRL_READ((&sInBufCtrl), &u32Temp);
					
			DPWM_WRITE_INDATA(DPWM, u32Temp & 0xFFFF);
			DPWM_WRITE_INDATA(DPWM, u32Temp >> 16 & 0xFFFF);
			
			u8Count += 2;
		}
		else
		{
			DPWM_WRITE_INDATA(DPWM, 0x0000);
			DPWM_WRITE_INDATA(DPWM, 0x0000);
			
			u8Count += 2;
		}
	}
}

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
	DPWM_SET_CLKSET(DPWM, DPWM_CLKSET_512FS);
	DPWM_SetSampleRate(u32SampleRate); //Set sample rate
	
	DPWM_ENABLE_FIFOTHRESHOLDINT(DPWM, 8);
  DPWM_SET_FIFODATAWIDTH(DPWM, DPWM_FIFO_DATAWIDTH_16BITS);
	DPWM_DISABLE_BIQUAD(DPWM);

	// Start DPWM */
	DPWM_CLEAR_FIFO(DPWM);
	DPWM_START_PLAY(DPWM);
	DPWM_ENABLE_DRIVER(DPWM);
	NVIC_EnableIRQ(DPWM_IRQn);
}
// UART0 ======================================================================================================== 
#define UART0_BAUDRATE (115200)
#define UART0_PIN_MASK (SYS_GPB_MFPH_PB8MFP_Msk|SYS_GPB_MFPH_PB9MFP_Msk)
#define UART0_PIN      (SYS_GPB_MFPH_PB8MFP_UART0_TXD|SYS_GPB_MFPH_PB9MFP_UART0_RXD)

void UART0_Initiate(void)
{
	// Enable UART module clock 
	CLK_EnableModuleClock(UART0_MODULE);
	// Select UART module clock source as HXT and UART module clock divider as 1
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));
	// Reset UART0 module
	SYS_ResetModule(UART0_RST);
	// Configure UART0 and set UART0 Baud rate
	UART_Open(UART0, UART0_BAUDRATE);
	// Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8)
	SYS->GPB_MFPH = (SYS->GPB_MFPH&~UART0_PIN_MASK)|UART0_PIN;
}
/*** (C) COPYRIGHT 2014~2015 Nuvoton Technology Corp. ***/
