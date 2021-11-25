/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/12/28 10:04a $
 * @brief	Please refer readme.txt
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
 
#include <stdio.h>
#include "Platform.h"
#include "ConfigSysClk.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/ 
void System_Idle(void);
void UART0_Init(void);
void VAD_Init(void);
void VAD_Start(void);

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int main()
{
	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();
	// Initiate UART0 for printf
    UART0_Init();
	// Initiate voice active detection.
	VAD_Init();
	
	printf("\r\n+-------------------------------------------------+\r\n");
	printf("|           VAD Wake-up demo Sample Code          |\r\n");
	printf("+-------------------------------------------------+\r\n");
	printf("  >> Please press 'Enter' to enter to power-down... \r\n");
	getchar();
	printf("  >> Then, user could wake-up chip via VAD(DMIC CH0).\r\n");	
	
	// Start to voice detection.
	VAD_Start();
	
	while(1)
	{
		// System enter power-down(idle) mode	
		System_Idle();
		printf("  >> System wake-up via VAD.\r\n");
		NVIC_DisableIRQ(VAD_IRQn);
		
		printf("  >> Please press 'Space' to enter to power-down... \r\n");
		getchar();
		NVIC_EnableIRQ(VAD_IRQn);	
	}
}

// Voice Active Detection(VAD) = = = = = = = = = = = = = = = = = = = = = = = = = = =
#define VAD_DETECT_SAMPLERATE      (8000)
#define VAD_DMIC_PIN_MASK          (SYS_GPD_MFPL_PD5MFP_Msk|SYS_GPD_MFPL_PD6MFP_Msk)
#define VAD_DMIC_PIN               (SYS_GPD_MFPL_PD5MFP_DMIC_CLK0|SYS_GPD_MFPL_PD6MFP_DMIC_DAT0)

void VAD_Init(void)
{
	// (1) Enable DMIC for VAD(Voice active detection)
	{
		// Select DMIC CLK source from PLL. 
		CLK_SetModuleClock(DMIC_MODULE, CLK_CLKSEL2_DMICSEL_HIRC, MODULE_NoMsk);	
		// Enable DMIC clock. 
		CLK_EnableModuleClock(DMIC_MODULE);
		// DPWM IPReset. 
		SYS_ResetModule(DMIC_RST);
		// Set channel's latch data falling type. 
		DMIC_SET_LATCHDATA(DMIC,DMIC_CTL_LATCHDATA_CH01F);
		// Enable DMIC FIFO threshold interrupt.
		DMIC_ENABLE_FIFOTHRESHOLDINT(DMIC,8);
		// GPIO multi-function.	
		SYS->GPD_MFPL = (SYS->GPD_MFPL & ~VAD_DMIC_PIN_MASK) | VAD_DMIC_PIN;
	}
	// (2) Enable VAD and input its detect parameter.
	{
		// Enable VAD down sample rate 64.
		VAD_ENABLE_DOWMSAMPLE(VAD,VAD_SINC_CTL_DOWNSAMPLE_64);
		// Set detect sample rate.
		VAD_SetSampleRate(VAD,VAD_DETECT_SAMPLERATE);
		// Writer BIQ coefficient(a0,a1,b0,b1,b2)
		VAD_WriteBIQCoeff(VAD,VAD_COEFF_A1,0xC715);
		VAD_WriteBIQCoeff(VAD,VAD_COEFF_A2,0x19A0);
		VAD_WriteBIQCoeff(VAD,VAD_COEFF_B0,0x1CA3);
		VAD_WriteBIQCoeff(VAD,VAD_COEFF_B1,0xC6BB);
		VAD_WriteBIQCoeff(VAD,VAD_COEFF_B2,0x1CA3);
		// Enable VAD BIQ filter.
		VAD_ENABLE_BIQ(VAD);
		// Set short term attack time.
		VAD_SET_STAT(VAD,VAD_CTL0_STAT_8MS);
		// Set long term attack time
		VAD_SET_LTAT(VAD,VAD_CTL0_LTAT_256MS);	
		// Set short term power threshold.
		VAD_SET_STTHRE(VAD,VAD_POWERTHRE_50DB,VAD_POWERTHRE_40DB);
		// Set deviation threshold.
		VAD_SET_DEVTHRE(VAD,VAD_POWERTHRE_50DB);		
	}
}
void VAD_Start(void)
{
	NVIC_EnableIRQ(VAD_IRQn);
	VAD_ENABLE(VAD);
}
void VAD_Stop(void)
{
	VAD_DISABLE(VAD);
	NVIC_DisableIRQ(VAD_IRQn);
}
void VAD_IRQHandler() 
{
	DMIC->CTL &= ~(DMIC_CTL_CHEN3_Msk | DMIC_CTL_CHEN2_Msk | DMIC_CTL_CHEN1_Msk | DMIC_CTL_CHEN0_Msk);
	VAD->SINC_CTL |= (VAD_SINC_CTL_ACTCL_Msk | VAD_SINC_CTL_SW_Msk | VAD_SINC_CTL_ACTCL_Msk); //set to clear 	
	while ((VAD->STATUS0 & VAD_STATUS0_ACTIVE_Msk) == VAD_STATUS0_ACTIVE_Msk);

	VAD->SINC_CTL &= ~(VAD_SINC_CTL_ACTCL_Msk | VAD_SINC_CTL_SW_Msk | VAD_SINC_CTL_ACTCL_Msk); //clear to return to normal operation
}
// UART0 ======================================================================================================== 
#define UART0_BAUDRATE     (115200)
#define UART0_MSC_PIN_MASK (SYS_GPB_MFPH_PB8MFP_Msk|SYS_GPB_MFPH_PB9MFP_Msk)
#define UART0_MSC_PIN      (SYS_GPB_MFPH_PB8MFP_UART0_TXD|SYS_GPB_MFPH_PB9MFP_UART0_RXD)

void UART0_Init(void)
{
    // Enable UART module clock 
    CLK_EnableModuleClock(UART0_MODULE);
    // Select UART module clock source as PLL and UART module clock divider as 1
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));
	// Configure UART0 and set UART0 Baud rate
    UART_Open(UART0, UART0_BAUDRATE);
    // Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8)
    SYS->GPB_MFPH = (SYS->GPB_MFPH&~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk))
	|(SYS_GPB_MFPH_PB8MFP_UART0_TXD|SYS_GPB_MFPH_PB9MFP_UART0_RXD);
}
void System_Idle(void)
{
    // Unlock protected registers
    SYS_UnlockReg();
	// System enter to idle mode.
	CLK_Idle();
    // Lock protected registers. 
    SYS_LockReg();		
}

