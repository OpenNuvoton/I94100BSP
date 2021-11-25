/**************************************************************************//**
 * @file     main.c
 * @version  V0.1
 * $Revision: 1 $
 * $Date: 27/03/24 11:20a $
 * @brief
 *           Demonstrate SPI data transfer.
 *           SPI0 will be configured as Master mode and SPI1 will be configured as Slave mode.
 *           SPI0(master) transfer via interrupt and SPI1(slave) transfer via main loop.
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "ConfigSysClk.h"

#define TEST_COUNT           (64)

/* Function prototype declaration */
void SPI_Init(void);
void UART_Init(void);
void SpiLoopTest(void);

/* Global variable declaration */
uint32_t g_au32MasterTxBuffer[TEST_COUNT];
uint32_t g_au32SlaveTxBuffer[TEST_COUNT];
uint32_t g_au32MasterRxBuffer[TEST_COUNT];
uint32_t g_au32SlaveRxBuffer[TEST_COUNT];
uint32_t g_u32MasterRxCount,g_u32MasterTxCount,g_u32SlaveRxCount,g_u32SlaveTxCount;

int main(void)
{
	uint32_t u32DataCount;

	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	// UART Initial.
	UART_Init();

	/* Init SPI */
	SPI_Init();

	/* Source data initiation */
	for(u32DataCount = 0; u32DataCount < TEST_COUNT; u32DataCount++)
	{
			g_au32MasterTxBuffer[u32DataCount] = 0x55000000 | (u32DataCount + 1);
			g_au32SlaveTxBuffer[u32DataCount] = 0xAA000000 | (u32DataCount + 1);
	}

	printf("\n\n");
	printf("+--------------------------------------------------------------+\n");
	printf("|                     SPI Sample Code                          |\n");
	printf("+--------------------------------------------------------------+\n");
	printf("\n");
	printf("Configure SPI0 as a master and SPI1 as a slave.\n");
	printf("Bit length of a transaction: 32\n");
	printf("The I/O connection for SPI0/SPI1 loopback:\n");
	printf("    SPI0_SS  (PA6) <--> SPI1_SS(PD5)\n    SPI0_CLK(PA5)  <--> SPI1_CLK(PD4)\n");
	printf("    SPI0_MISO(PA4) <--> SPI1_MISO(PD3)\n    SPI0_MOSI(PA3) <--> SPI1_MOSI(PD2)\n\n");
	printf("Please connect SPI0 with SPI1, and press any key to start transmission ...\n");
	getchar();
	printf("\nSPI0/1 Loop test  ");

	SpiLoopTest();

	printf("\nExit SPI driver sample code.\n");

	/* Close SPI0 */
	SPI_Close(SPI0);
	/* Close SPI1 */
	SPI_Close(SPI1);

	while(1);
}

void UART_Init(void)
{
	/* Select HXT as the clock source of UART0 */
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));

	/* Enable UART peripheral clock */
	CLK_EnableModuleClock(UART0_MODULE);
	/* Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8) */
	SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk)) | (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD);

	/* Reset UART0 module */
	SYS_ResetModule(UART0_RST);
	/* Configure UART0: 115200, 8-bit word, no parity bit, 1 stop bit. */
	UART_Open(UART0, 115200);
}

void SPI_Init(void)
{
	/* Select PCLK0 as the clock source of SPI0 */
	CLK_SetModuleClock(SPI0_MODULE, CLK_CLKSEL2_SPI0SEL_PCLK0, MODULE_NoMsk);
	/* Select PCLK1 as the clock source of SPI1 */
	CLK_SetModuleClock(SPI1_MODULE, CLK_CLKSEL2_SPI1SEL_PCLK1, MODULE_NoMsk);

	/* Enable SPI0 peripheral clock */
	CLK_EnableModuleClock(SPI0_MODULE);
	/* Enable SPI1 peripheral clock */
	CLK_EnableModuleClock(SPI1_MODULE);
	/* Set SPI0 multi-function pins */
	SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA3MFP_Msk | SYS_GPA_MFPL_PA4MFP_Msk | SYS_GPA_MFPL_PA5MFP_Msk | SYS_GPA_MFPL_PA6MFP_Msk)) | 
	(SYS_GPA_MFPL_PA5MFP_SPI0_CLK | SYS_GPA_MFPL_PA4MFP_SPI0_MISO0 | SYS_GPA_MFPL_PA6MFP_SPI0_SS0 | SYS_GPA_MFPL_PA3MFP_SPI0_MOSI0);

	/* Configure SPI1 related multi-function pins. GPD[5:2] : SPI1_CLK, SPI1_MISO, SPI1_MOSI, SPI1_SS. */
	 SYS->GPD_MFPL = (SYS->GPD_MFPL & ~(SYS_GPD_MFPL_PD2MFP_Msk | SYS_GPD_MFPL_PD3MFP_Msk | SYS_GPD_MFPL_PD4MFP_Msk | SYS_GPD_MFPL_PD5MFP_Msk)) | 
	(SYS_GPD_MFPL_PD5MFP_SPI1_SS | SYS_GPD_MFPL_PD2MFP_SPI1_MOSI | SYS_GPD_MFPL_PD3MFP_SPI1_MISO | SYS_GPD_MFPL_PD4MFP_SPI1_CLK);
	/* Configure SPI0 */
	/* Reset module */
	SYS_ResetModule(SPI0_RST);
	/* Configure SPI0 as a master, SPI clock rate 2 MHz, */
	/* clock idle low, 32-bit transaction, drive output on falling clock edge and latch input on rising edge. */
	SPI_Open(SPI0, SPI_MASTER, SPI_MODE_0, 32, 2000000);
	/* Enable the automatic hardware slave selection function. Select the SPI0_SS pin and configure as low-active. */
	SPI_EnableAutoSS(SPI0, SPI_SS0, SPI_SS_ACTIVE_LOW);
	/* Config Suspend cycle */
	SPI_SET_SUSPEND_CYCLE(SPI0,5);
	/* Enable SPI0 interrupt */
	SPI_EnableInt(SPI0, SPI_UNIT_INT_MASK);
	/* Enable NVIC of SPI0 control */
	NVIC_EnableIRQ(SPI0_IRQn);

	/* Configure SPI1 */
	/* Reset module */
	SYS_ResetModule(SPI1_RST);
	/* Configure SPI1 as a slave, clock idle low, 32-bit transaction, drive output on falling clock edge and latch input on rising edge. */
	/* Configure SPI1 as a low level active device. SPI peripheral clock rate = f_PCLK1 */
	SPI_Open(SPI1, SPI_SLAVE, SPI_MODE_0, 32, NULL);
}

void SpiLoopTest(void)
{
	BOOL bError = FALSE;
	UINT32 u32TestCycle;
	
	for(u32TestCycle = 0; u32TestCycle < 5000; u32TestCycle++)
	{
		if((u32TestCycle & 0x1FF) == 0)
			putchar('.');

		/* Initiate RX buffer & RX/TX counter for next test */
		g_u32MasterRxCount = g_u32MasterTxCount = g_u32SlaveRxCount = g_u32SlaveTxCount = 0;
		memset(g_au32SlaveRxBuffer, '\0', sizeof(g_au32SlaveRxBuffer));
		memset(g_au32MasterRxBuffer, '\0', sizeof(g_au32MasterRxBuffer));

		/* Check TX FULL flag and TX data count */
		while((SPI_GET_TX_FIFO_FULL_FLAG(SPI1) == 0) && (g_u32SlaveTxCount < TEST_COUNT))
		{
			/* Write to TX FIFO */
			SPI_WRITE_TX(SPI1, g_au32SlaveTxBuffer[g_u32SlaveTxCount++]);
		}

		/* Write to TX FIFO */
		SPI_WRITE_TX(SPI0, g_au32MasterTxBuffer[g_u32MasterTxCount++]);

		/* SPI1(slave) RX & TX process */
		while(g_u32MasterRxCount<TEST_COUNT || g_u32SlaveRxCount<TEST_COUNT)
		{
			/* Check RX EMPTY flag */
			if(SPI_GET_RX_FIFO_EMPTY_FLAG(SPI1) == 0)
			{
				/* Read RX FIFO */
				g_au32SlaveRxBuffer[g_u32SlaveRxCount++] = SPI_READ_RX(SPI1);
			}	
			/* Check TX FULL flag and TX data count */
			if((SPI_GET_TX_FIFO_FULL_FLAG(SPI1) == 0) && (g_u32SlaveTxCount < TEST_COUNT))
			{
				/* Write to TX FIFO */
				SPI_WRITE_TX(SPI1, g_au32SlaveTxBuffer[g_u32SlaveTxCount++]);
			}
		}

		if( (memcmp(g_au32SlaveRxBuffer,g_au32MasterTxBuffer, sizeof(g_au32SlaveRxBuffer)) != 0) ||
		(memcmp(g_au32MasterRxBuffer,g_au32SlaveTxBuffer, sizeof(g_au32MasterRxBuffer)) != 0))
		{
			bError = TRUE;
			break;
		}

	}

	if(bError)
	{
		printf(" [FAIL]\n");
	}
	else
	{
		printf(" [PASS]\n");
	}
}

void SPI0_IRQHandler(void)
{
	/* Check RX EMPTY flag */
	if(SPI_GET_RX_FIFO_EMPTY_FLAG(SPI0) == 0)
	{
		/* Read RX FIFO */
		g_au32MasterRxBuffer[g_u32MasterRxCount++] = SPI_READ_RX(SPI0);
	}		
	/* Check TX FULL flag and TX data count */
	if((SPI_GET_TX_FIFO_FULL_FLAG(SPI0) == 0) && (g_u32MasterTxCount < TEST_COUNT))
	{
		/* Write to TX FIFO */
		SPI_WRITE_TX(SPI0, g_au32MasterTxBuffer[g_u32MasterTxCount++]);
	}
	SPI_CLR_UNIT_TRANS_INT_FLAG(SPI0);
}
