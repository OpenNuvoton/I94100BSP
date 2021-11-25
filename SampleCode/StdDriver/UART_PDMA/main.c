/****************************************************************************
 * @file     main.c
 * @version  V1.00
 * @brief    Demonstrate UART transmit and receive function with PDMA
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "Platform.h"
#include "ConfigSysClk.h"
#define ENABLE_PDMA_INTERRUPT 1
#define PDMA_TEST_LENGTH 100

/*---------------------------------------------------------------------------------------------------------*/
/* Global variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
static uint8_t g_u8Tx_Buffer[PDMA_TEST_LENGTH];

volatile uint32_t g_u32IsTestOver = 0;

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void);
void PDMA_IRQHandler(void);
void UART_PDMATest(void);

/**
 * @brief       DMA IRQ
 *
 * @param       None
 *
 * @return      None
 *
 * @details     The DMA default IRQ, declared in startup .s
 */
void PDMA_IRQHandler(void)
{
	uint32_t status = PDMA_GET_INT_STATUS();

	if(status & PDMA_INTSTS_ABTIF_Msk)		/* abort */
	{  
		/* Check if channel 0 has abort error */
		if(PDMA_GET_ABORT_STS() & PDMA_CH0_MASK)
				g_u32IsTestOver = 2;
		/* Clear abort flag of channel 0 */
		PDMA_CLR_ABORT_FLAG(PDMA_CH0_MASK);
	} 
	else if(status & PDMA_INTSTS_TDIF_Msk) /* done */
	{  
		/* Check transmission of channel 0 has been transfer done */
		if(PDMA_GET_TD_STS() & PDMA_CH0_MASK)
				g_u32IsTestOver = 1;
		/* Clear transfer done flag of channel 0 */
		PDMA_CLR_TD_FLAG(PDMA_CH0_MASK);
	} 
	else
		printf("unknown interrupt !!\n");
}

void UART0_Init()
{
	/* Enable UART module clock */
	CLK_EnableModuleClock(UART0_MODULE);

	/* Select UART module clock source as HXT and UART module clock divider as 1 */
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));
	/* Reset UART module */
	SYS_ResetModule(UART0_RST);
	UART_Open(UART0, 115200);

	/* Enable PDMA clock source */
	CLK_EnableModuleClock(PDMA_MODULE);

	/* Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8) */
	SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk);
	SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD);
}

void PDMA_Init(void)
{
	/* Open PDMA Channel */
	PDMA_Open(PDMA_CH0_MASK); // Channel 0 for UART0 TX

	// Select basic mode
	PDMA_SetTransferMode(0, PDMA_UART0_TX, 0, 0);
	
	// Set data width and transfer count
	PDMA_SetTransferCnt(0, PDMA_WIDTH_8, PDMA_TEST_LENGTH);

	//Set PDMA Transfer Address
	PDMA_SetTransferAddr(0, (uint32_t)&g_u8Tx_Buffer[0], PDMA_SAR_INC, (uint32_t)&UART0->DAT, PDMA_DAR_FIX);

	//Select Single Request
	PDMA_SetBurstType(0, PDMA_REQ_SINGLE, 0);

	PDMA_EnableInt(0, PDMA_INT_TRANS_DONE);
	NVIC_EnableIRQ(PDMA_IRQn);
	g_u32IsTestOver = 0;
}

/*---------------------------------------------------------------------------------------------------------*/
/* MAIN function                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/

int main(void)
{
	SYSCLK_INITIATE();

	/* Init UART0 for printf */
	UART0_Init();

	/*---------------------------------------------------------------------------------------------------------*/
	/* SAMPLE CODE                                                                                             */
	/*---------------------------------------------------------------------------------------------------------*/

	printf("\n\nCPU @ %dHz\n", SystemCoreClock);

	UART_PDMATest();

	while(1);
}

/*---------------------------------------------------------------------------------------------------------*/
/*  UART PDMA Test                                                                                     */
/*---------------------------------------------------------------------------------------------------------*/
void UART_PDMATest()
{
	uint32_t i;

	printf("+-----------------------------------------------------------+\n");
	printf("|  UART PDMA Test                                           |\n");
	printf("+-----------------------------------------------------------+\n");
	printf("|  Description :                                            |\n");
	printf("|    The sample code will demo UART0 with PDMA function.    |\n");
	printf("|    Please connect to PC terminal and send data            |\n");
	printf("+-----------------------------------------------------------+\n");
	printf("Please press any key to start test. \n\n");

	getchar();

	for (i=0; i<PDMA_TEST_LENGTH; i++) 
	{
		g_u8Tx_Buffer[i] = i;
	}

	PDMA_Init();

	// Enable UART TX DMA service 
	UART_ENABLE_INT(UART0, UART_INTEN_TXPDMAEN_Msk);
	//UART0->INTEN |= UART_INTEN_TXPDMAEN_Msk;

	while(g_u32IsTestOver == 0);

	if (g_u32IsTestOver == 1)
	printf("\ntest done...\n");
	else if (g_u32IsTestOver == 2)
	printf("\ntarget abort...\n");

	UART0->INTEN &= ~UART_INTEN_TXPDMAEN_Msk;

	PDMA_Close();

	printf("\nUART PDMA test Pass.\n");
}
