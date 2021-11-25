/**************************************************************************//**
 * @file     main.c
 * @version  V0.10
 * $Revision: 1 $
 * $Date: 18/05/15 10:04a $
 * @brief
 *           Demonstrate how I2S works in Master mode.
 *           This sample code needs to work with I2S_Slave.
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "BUFCTRL.h"
#include "ConfigSysClk.h"

#define DEMO_DATA_COUNT (256)

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/   
void UART0_Initiate(void);
void I2S0_Master_Initiate(S_BUFCTRL* psInBufCtrl,S_BUFCTRL* psOutBufCtrl);
void I2S0_Master_Start(void);

/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables and constants                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
volatile S_BUFCTRL sInBufCtrl,sOutBufCtrl; // Buffer control handler.
int32_t ai32InBuf[DEMO_DATA_COUNT];        // Buffer array: provide I2S_Master receiver data. 
int32_t ai32OutBuf[DEMO_DATA_COUNT];       // Buffer array: provide I2S_Master send data. 

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
	UINT32 u32i;
	
	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();
	
	// These defines are from  BUFCTRL.h for buffer control in this samle. 
	// Buffer control handler configuration. 
	BUFCTRL_CFG((&sInBufCtrl),ai32InBuf,sizeof(ai32InBuf)/sizeof(uint32_t));
	BUFCTRL_CFG((&sOutBufCtrl),ai32OutBuf,sizeof(ai32OutBuf)/sizeof(uint32_t));
	
	// Initiate UART0 for print message.
	UART0_Initiate();
	
	// Initiate I2S0 master
	I2S0_Master_Initiate((S_BUFCTRL*)(&sInBufCtrl),(S_BUFCTRL*)(&sOutBufCtrl));

	printf("+-----------------------------------------------------------+\n");
	printf("|            I2S Driver Sample Code (master mode)           |\n");
	printf("+-----------------------------------------------------------+\n");
	printf("I2S configuration:\n");
	printf("Sample rate 48 KHz\n");
	printf("Word width 32 bits\n");
	printf("Stereo mode\n");
	printf("I2S format\n");
	printf("TX value: 0x5A5A0000, 0x5A5A0001, ..., 0x5A5A00FF\n");
	printf("The I/O connection for I2S0:\n");
	printf("Master PD6(BCLK) < - > Slave PD6(BCLK)\n");
	printf("Master PD5(DO) < - > Slave PD4(DI)\n");
	printf("Master PD4(DI) < - > Slave PD5(DO)\n");
	printf("Master PD3(LRCK) < - > Slave PD3(LRCK)\n");
	printf("Press any key to start ...");
	getchar();
	printf("\n");
	
	// Set send data into output buffer.
	for( u32i=0; u32i<DEMO_DATA_COUNT; u32i++ )
	{
		BUFCTRL_WRITE(((S_BUFCTRL*)(&sOutBufCtrl)),(0x5A5A0000+u32i));
		ai32InBuf[u32i] = 0xFFFFFFFF;
	}
	
	// Start send data from output buffer and receive data into input buffer.
	I2S0_Master_Start();
	
	// Wait until input buffer receive data counts = DEMO_DATA_COUNT.
	while(BUFCTRL_GET_COUNT(((&sInBufCtrl))) < DEMO_DATA_COUNT);
	
	// Verity receive data and compare with send data.
	for( u32i=0; u32i<DEMO_DATA_COUNT; u32i++ )
	{
		printf("Received Datap[%d]:  %x.\n", u32i, ai32InBuf[u32i]);
	}
	
	printf("Demo End...\n\n");
	while(1);
}

// I2S_Master ===================================================================================================
#define I2S_MASTER_PIN_MASK     (SYS_GPD_MFPL_PD2MFP_Msk|SYS_GPD_MFPL_PD3MFP_Msk|SYS_GPD_MFPL_PD4MFP_Msk|SYS_GPD_MFPL_PD5MFP_Msk|SYS_GPD_MFPL_PD6MFP_Msk)
#define I2S_MASTER_PIN          (SYS_GPD_MFPL_PD2MFP_I2S0_MCLK|SYS_GPD_MFPL_PD3MFP_I2S0_LRCK|SYS_GPD_MFPL_PD4MFP_I2S0_DI|SYS_GPD_MFPL_PD5MFP_I2S0_DO|SYS_GPD_MFPL_PD6MFP_I2S0_BCLK)
#define I2S_MASTER_SAMPLE_RATE  (48000)

// Provide I2S0 master's buffer control.
S_BUFCTRL* g_psI2S0_InBufCtrl = NULL;
S_BUFCTRL* g_psI2S0_OutBufCtrl = NULL;      

void I2S0_Master_Initiate(S_BUFCTRL* psInBufCtrl,S_BUFCTRL* psOutBufCtrl)
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
	I2S_Open(I2S0, I2S_MASTER, I2S_MASTER_SAMPLE_RATE, I2S_DATABIT_32, I2S_TDMCHNUM_2CH, I2S_STEREO, I2S_FORMAT_I2S);
	// I2S0 Configuration
	I2S_SET_PCMSYNC(I2S0, I2S_PCMSYNC_BCLK);
	I2S_SET_MONO_RX_CHANNEL(I2S0, I2S_MONO_RX_RIGHT);
	I2S_SET_STEREOORDER(I2S0, I2S_ORDER_EVENLOW);
	// Set channel width.
	I2S_SET_CHWIDTH(I2S0, I2S_CHWIDTH_32);
	// Set FIFO threshold.
	I2S_SET_TXTH(I2S0, I2S_FIFO_TX_LEVEL_WORD_8);
	I2S_SET_RXTH(I2S0, I2S_FIFO_RX_LEVEL_WORD_8);
	// Enable interrupt.
	I2S_ENABLE_INT(I2S0, I2S_TXTH_INT_MASK|I2S_RXTH_INT_MASK);
	// Enable I2S's NVIC. 
	NVIC_EnableIRQ(I2S0_IRQn);
	// Set buffer control pointer.
	g_psI2S0_InBufCtrl = psInBufCtrl;
	g_psI2S0_OutBufCtrl = psOutBufCtrl;
}

void I2S0_Master_Start(void)
{
	UINT32 u32Data = 0;
	
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

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
