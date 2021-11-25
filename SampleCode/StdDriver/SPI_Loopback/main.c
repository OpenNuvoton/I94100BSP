/******************************************************************************
 * @file     main.c
 * @version  V1
 * $Revision: 1 $
 * $Date: 17/03/14 10:04a $
 * @brief
 *           Implement SPI Master loop back transfer.
 *           This sample code needs to connect SPI0_MISO0 pin and SPI0_MOSI0 pin together.
 *           It will compare the received data with transmitted data.
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "Platform.h"
#include "ConfigSysClk.h"

#define TEST_COUNT  (64)

uint32_t g_au32SourceData[TEST_COUNT];
uint32_t g_au32DestinationData[TEST_COUNT];

/* Function prototype declaration */
void SPI_Init(void);
void UART_Init(void);

/* ------------- */
/* Main function */
/* ------------- */
int main(void)
{
	uint32_t u32DataCount, u32TestCount, u32Err;

	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	// UART Initial.
	UART_Init();

	/* Init SPI */
	SPI_Init();

	printf("\n\n");
	printf("+------------------------------------------------------------------+\n");
	printf("|                     SPI Driver Sample Code                       |\n");
	printf("+------------------------------------------------------------------+\n");
	printf("\nThis sample code demonstrates SPI0 self loop back data transfer.\n");
	printf(" SPI0 configuration:\n");
	printf("     Master mode; data width 32 bits.\n");
	printf(" I/O connection:\n");
	printf("     PA.3 SPI0_MOSI0 <--> PA.4 SPI0_MISO0 \n");

	printf("\nSPI0 Loopback test ");

	/* set the source data and clear the destination buffer */
	for(u32DataCount = 0; u32DataCount < TEST_COUNT; u32DataCount++)
	{
		g_au32SourceData[u32DataCount] = u32DataCount;
		g_au32DestinationData[u32DataCount] = 0;
	}

	u32Err = 0;
	for(u32TestCount = 0; u32TestCount < 0x1000; u32TestCount++)
	{
		/* set the source data and clear the destination buffer */
		for(u32DataCount = 0; u32DataCount < TEST_COUNT; u32DataCount++)
		{
			g_au32SourceData[u32DataCount]++;
			g_au32DestinationData[u32DataCount] = 0;
		}

		u32DataCount = 0;

		if((u32TestCount & 0x1FF) == 0)
		{
				putchar('.');
		}

		while(1)
		{
			/* Write to TX register */
			SPI_WRITE_TX(SPI0, g_au32SourceData[u32DataCount]);
			// Check tx fifo empty flag
			while(SPI_GET_TX_FIFO_EMPTY_FLAG(SPI0) == 0);
			// Check rx fifo empty flag
			while(SPI_GET_RX_FIFO_EMPTY_FLAG(SPI0));
			/* Read received data */
			g_au32DestinationData[u32DataCount] = SPI_READ_RX(SPI0);
			u32DataCount++;
			if(u32DataCount == TEST_COUNT)
					break;
		}

		/*  Check the received data */
		for(u32DataCount = 0; u32DataCount < TEST_COUNT; u32DataCount++)
		{
			if(g_au32DestinationData[u32DataCount] != g_au32SourceData[u32DataCount])
					u32Err = 1;
		}

		if(u32Err)
			break;
	}

	if(u32Err)
		printf(" [FAIL]\n\n");
	else
		printf(" [PASS]\n\n");

	/* Close SPI0 */
	SPI_Close(SPI0);

	while(1);
}

void UART_Init()
{
	/* Select HXT as the clock source of UART0 */
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));

	/* Enable UART peripheral clock */
	CLK_EnableModuleClock(UART0_MODULE);

	/* Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8) */
	SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk)) | (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD);

	/* Configure UART0: 115200, 8-bit word, no parity bit, 1 stop bit. */
	UART_Open(UART0, 115200);
}

void SPI_Init(void)
{
	/* Select PCLK0 as the clock source of SPI0 */
	CLK_SetModuleClock(SPI0_MODULE, CLK_CLKSEL2_SPI0SEL_PCLK0, MODULE_NoMsk);

	/* Enable SPI0 peripheral clock */
	CLK_EnableModuleClock(SPI0_MODULE);

	/* Set SPI0 multi-function pins */
	SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA3MFP_Msk | SYS_GPA_MFPL_PA4MFP_Msk | SYS_GPA_MFPL_PA5MFP_Msk | SYS_GPA_MFPL_PA6MFP_Msk)) | 
	(SYS_GPA_MFPL_PA5MFP_SPI0_CLK | SYS_GPA_MFPL_PA4MFP_SPI0_MISO0 | SYS_GPA_MFPL_PA6MFP_SPI0_SS0 | SYS_GPA_MFPL_PA3MFP_SPI0_MOSI0);

	/* Reset module */
	SYS_ResetModule(SPI0_RST);

	/* Configure as a master, clock idle low, 32-bit transaction, drive output on falling clock edge and latch input on rising edge. */
	/* Set IP clock divider. SPI clock rate = 2MHz */
	SPI_Open(SPI0, SPI_MASTER, SPI_MODE_0, 32, 2000000);

	/* Enable SPI interface */
	SPI_ENABLE(SPI0);
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
