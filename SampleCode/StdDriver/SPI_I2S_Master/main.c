/**************************************************************************//**
 * @file     main.c
 * @version  V0.10
 * $Revision: 1 $
 * $Date: 17/04/11 10:04a $
 * @brief
 *           Configure SPI1 as SPI_I2S Master mode and demonstrate how SPI_I2S works in Master mode.
 *           This sample code needs to work with SPI_I2S_Slave.
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "ConfigSysClk.h"

uint32_t g_u32TxValue;
uint32_t g_u32DataCount;

/* Function prototype declaration */
void UART_Init(void);
void SPI_Init(void);

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
	uint32_t u32RxValue1, u32RxValue2;

	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	UART_Init();

	SPI_Init();

	printf("+-----------------------------------------------------------+\n");
	printf("|            SPI_I2S Driver Sample Code (master mode)           |\n");
	printf("+-----------------------------------------------------------+\n");
	printf("  SPI_I2S configuration:\n");
	printf("      Sample rate 16 kHz\n");
	printf("      Word width 16 bits\n");
	printf("      Stereo mode\n");
	printf("      SPI_I2S format\n");
	printf("      TX value: 0x55005501, 0x55025503, ..., 0x55FE55FF, wraparound\n");
	printf("  The I/O connection for SPI_I2S (SPI1):\n");
	printf("      SPI1_I2S_LRCLK (PD5)\n   SPI1_I2S_BCLK(PD4)\n");
	printf("      SPI1_I2S_DI (PD3) to Slave DO pin\n      SPI1_I2S_DO (PD2) to Slave DI pin.\n\n");
	printf("  NOTE: Connect with a SPI_I2S slave device.\n");
	printf("        Run the slave demo first to wait for master transmission, and then start master demo.\n");
	printf("        This sample code will transmit a TX value 50000 times, and then change to the next TX value.\n");
	printf("        When TX value or the received value changes, the new TX value or the current TX value and the new received value will be printed.\n");
	printf("  Press any key to start ...");
	getchar();
	printf("\n");

	/* Master mode, 16-bit word width, stereo mode, I2S format. Set TX and RX FIFO threshold to middle value. */
	SPI_I2SOpen(SPI1, SPI_I2SMASTER, 16000, SPI_I2SDATABIT_16, SPI_I2SSTEREO, SPI_I2SFORMAT_I2S);

	/* Initiate data counter */
	g_u32DataCount = 0;
	/* Initiate TX value and RX value */
	g_u32TxValue = 0x55005501;
	u32RxValue1 = 0;
	u32RxValue2 = 0;
	/* Enable TX threshold level interrupt */
	SPI_I2S_SET_TXTH(SPI1, SPI_I2S_FIFO_TX_LEVEL_4);
	SPI_I2SEnableInt(SPI1, SPI_I2S_TXTH_INT_MASK);
	SPI_I2S_RST_TX_FIFO(SPI1);
	SPI_I2S_RST_RX_FIFO(SPI1);
	NVIC_EnableIRQ(SPI1_IRQn);

	// Enable I2S.
	SPI_I2SEnableControl(SPI1);
	SPI_I2S_ENABLE_TX(SPI1);
	SPI_I2S_ENABLE_RX(SPI1);
	printf("Start SPI_I2S ...\nTX value: 0x%X\n", g_u32TxValue);

	while(1)
	{
		//if((SPI1->I2SSTS & SPI_I2SSTS_RXEMPTY_Msk) == 0)
				/* Check RX FIFO empty flag */
		if(!SPI_I2S_IS_RX_EMPTY(SPI1))
		{
			/* Read RX FIFO */
			u32RxValue2 = SPI_I2S_READ_RX_FIFO(SPI1);
			if(u32RxValue1 != u32RxValue2)
			{
				u32RxValue1 = u32RxValue2;
				/* If received value changes, print the current TX value and the new received value. */
				printf("TX value: 0x%X;  RX value: 0x%X\n", g_u32TxValue, u32RxValue1);
			}
		}
		if(g_u32DataCount >= 50000)
		{
			g_u32TxValue = 0x55005500 | ((g_u32TxValue + 0x00020002) & 0x00FF00FF); /* g_u32TxValue: 0x55005501, 0x55025503, ..., 0x55FE55FF */
			printf("TX value: 0x%X\n", g_u32TxValue);
			g_u32DataCount = 0;
		}
	}
}

void UART_Init()
{
	/* Select UART module clock source as HXT and UART module clock divider as 1 */
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));

	/* Enable peripheral clock */
	CLK_EnableModuleClock(UART0_MODULE);

	/* Set PD multi-function pins for UART0 RXD, TXD */
	SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk)) | (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD);

	/* Reset UART0 module */
	SYS_ResetModule(UART0_RST);
	
	/* Init UART0 to 115200-8n1 for printing messages */
	UART_Open(UART0, 115200);
}

void SPI_Init()
{
	/* Select PCLK1 as the clock source of SPI1(I2S0) */
	CLK_SetModuleClock(SPI1_MODULE, CLK_CLKSEL2_SPI1SEL_PCLK1, MODULE_NoMsk);

	/* Enable peripheral clock */
	CLK_EnableModuleClock(SPI1_MODULE);

	/* Set SPI1 multi-function pins */
	SYS->GPD_MFPL = (SYS->GPD_MFPL & ~(SYS_GPD_MFPL_PD2MFP_Msk | SYS_GPD_MFPL_PD3MFP_Msk | SYS_GPD_MFPL_PD4MFP_Msk | SYS_GPD_MFPL_PD5MFP_Msk)) | 
	(SYS_GPD_MFPL_PD4MFP_SPI1_CLK | SYS_GPD_MFPL_PD3MFP_SPI1_MISO | SYS_GPD_MFPL_PD5MFP_SPI1_SS | SYS_GPD_MFPL_PD2MFP_SPI1_MOSI);  
}

void SPI1_IRQHandler()
{
    /* Write 2 TX values to TX FIFO */
    SPI_I2S_WRITE_TX_FIFO(SPI1, g_u32TxValue);
    SPI_I2S_WRITE_TX_FIFO(SPI1, g_u32TxValue);
    g_u32DataCount += 2;
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
