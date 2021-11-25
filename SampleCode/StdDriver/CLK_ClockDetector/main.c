/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 19/11/22 10:04a $
 * @brief    Show the usage of clock fail detector.
 * @note
 * Copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "stdio.h"
#include "Platform.h"
#include "ConfigSysClk.h"

/*---------------------------------------------------------------------------------------------------------*/
/*  Clock Fail Detector IRQ Handler                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
void CLKF_IRQHandler(void)
{
	uint32_t u32Reg;

	u32Reg = CLK->CLKDSTS;

	if(u32Reg & CLK_CLKDSTS_HXTFIF_Msk)
	{
		/* HCLK is switched to HIRC automatically if HXT clock fail interrupt is happened */
		printf("HXT Clock is stopped! HCLK is switched to HIRC.\n");

		/* Disable HXT clock fail interrupt */
		CLK->CLKDCTL &= ~(CLK_CLKDCTL_HXTFDEN_Msk | CLK_CLKDCTL_HXTFIEN_Msk);

		/* Write 1 to clear HXT Clock fail interrupt flag */
		CLK->CLKDSTS = CLK_CLKDSTS_HXTFIF_Msk;
	}
}

void UART0_Init(void)
{   
	/* Enable UART module clock */
	CLK_EnableModuleClock(UART0_MODULE);

	/* Select UART module clock source as HIRC and UART module clock divider as 1 */
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_HIRC, CLK_CLKDIV0_UART0(1));

	/* Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8) */
	SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk);
	SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD);

	/* Reset UART module */
	SYS_ResetModule(UART0_RST);

	/* Configure UART0 and set UART0 baud rate */
	UART_Open(UART0, 115200);
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	/* Init UART0 for printf */
	UART0_Init();

	printf("\n\nCPU @ %d Hz\n", SystemCoreClock);
	printf("+-------------------------------------------------------------+\n");
	printf("|                Clock Detector Sample Code                   |\n");
	printf("+-------------------------------------------------------------+\n");
	printf("| 1. HXT clock fail interrupt will happen if HXT is stopped.  |\n");
	printf("|    HCLK clock source will be switched from HXT to HIRC.     |\n");
	printf("+-------------------------------------------------------------+\n");
	printf("\nStop HXT to test.\n\n");

	/* Enable clock output, select CLKO clock source as HCLK and set clock output frequency is HCLK/4.
		 HCLK clock source will be switched to HIRC if HXT stop and HCLK clock source is from HXT.
		 User can check if HCLK clock source is switched to HIRC by clock output pin output frequency.
	*/

	/* Output selected clock to CKO, CKO Clock = HCLK / 2^(1 + 1) */
	CLK_EnableCKO(CLK_CLKSEL1_CLKOSEL_HCLK, 1, 0);

	/* Set clock fail detector function enabled and interrupt enabled */
	CLK->CLKDCTL = CLK_CLKDCTL_HXTFDEN_Msk | CLK_CLKDCTL_HXTFIEN_Msk;
								
	/* Enable clock fail detector interrupt */
	NVIC_EnableIRQ(CLKF_IRQn);

	/* Wait for clock fail detector interrupt happened */
	while(1);
}
