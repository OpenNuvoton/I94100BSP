/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 4 $
 * $Date: 16/10/02 10:05a $
 * @brief    Please refer readme.txt
 * @note
 * Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "Platform.h"
#include "ConfigSysClk.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables and constants                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
extern uint32_t u32audioBegin, u32audioEnd;
int16_t *pi16AudioBegin;
int16_t *pi16AudioEnd;

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void UART0_Init(void);
void DPWM_Init(uint32_t u32SampleRate);

/*---------------------------------------------------------------------------------------------------------*/
/* Main function																						   */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	UART0_Init();
		
	DPWM_Init(8000);

	printf("DPWM sampling rate: %d Hz\r\n", DPWM_GetSampleRate());
	printf("\r\n");
		
	do
	{
		pi16AudioBegin = (int16_t *)&u32audioBegin;
		pi16AudioEnd = (int16_t *)&u32audioEnd;
		
		printf("Please input any key to play audio.\r\n\r\n");
		getchar();
		
		DPWM_START_PLAY(DPWM);
		DPWM_ENABLE_DRIVER(DPWM);
		DPWM_WriteMonotoStereo(pi16AudioBegin,(pi16AudioEnd-pi16AudioBegin ));
	}while( DPWM_IS_FIFOEMPTY(DPWM)== FALSE );
}

/*---------------------------------------------------------------------------------------------------------*/
/* DPWM interrupt handler                                                                                  */
/*---------------------------------------------------------------------------------------------------------*/
void DPWM_Init(uint32_t u32SampleRate)
{
	/* Enable DPWM module clock */
	CLK_EnableModuleClock(DPWM_MODULE);
	/* Set DPWM module clock */
	CLK_SetModuleClock(DPWM_MODULE, CLK_CLKSEL2_DPWMSEL_PCLK0, MODULE_NoMsk);
	/* Set APB0, APB1 clock source divider to 2. */
	//CLK_SetPCLKDivider(CLK_PCLKDIV_PCLK0DIV2);
	/* Reset IP */
	SYS_ResetModule(DPWM_RST);
	
	/* Set DPWM sampling rate */
	// Since PCLK = 96MHz, set CLKSEL to 500FS(500*8KHz) so the sample rate can be divisible.
	DPWM_SET_CLKSET(DPWM, DPWM_CLKSET_500FS);
	DPWM_SetSampleRate(u32SampleRate); //Set sample rate
	/* Set Datawidth */
	DPWM_SET_FIFODATAWIDTH(DPWM, DPWM_FIFO_DATAWIDTH_16BITS);
	
	/* GPIO multi-function.(GPD0:DPWM1_N,GPD1:DPWM1_P) */
	SYS->GPC_MFPH = (SYS->GPC_MFPH & ~(SYS_GPC_MFPH_PC13MFP_Msk|SYS_GPC_MFPH_PC12MFP_Msk))|(SYS_GPC_MFPH_PC13MFP_DPWM_LP|SYS_GPC_MFPH_PC12MFP_DPWM_LN);
	/* GPIO multi-function.(GPD5:DPWM0_N,GPD6:DPWM0_P) */
	SYS->GPC_MFPH = (SYS->GPC_MFPH & ~(SYS_GPC_MFPH_PC11MFP_Msk|SYS_GPC_MFPH_PC10MFP_Msk))|(SYS_GPC_MFPH_PC11MFP_DPWM_RP|SYS_GPC_MFPH_PC10MFP_DPWM_RN);
}

void UART0_Init(void)
{
	/* Enable peripheral clock */
	CLK_EnableModuleClock(UART0_MODULE);
	/* Peripheral clock source */
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));
	/* Reset UART module */
	SYS_ResetModule(UART0_RST);
	/* Configure UART0 and set UART0 baud rate */
	UART_Open(UART0, 115200);
	/* Set PB multi-function pins for UART0 RXD and TXD */
	SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk);
	SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD);
}
/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
