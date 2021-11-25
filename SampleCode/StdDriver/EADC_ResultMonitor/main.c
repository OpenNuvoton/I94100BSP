/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * @brief    Monitor the conversion result of channel 2 by the digital compare function.
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "ConfigSysClk.h"
#include "Platform.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables and constants                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
volatile uint32_t g_u32AdcCmp0IntFlag;
volatile uint32_t g_u32AdcCmp1IntFlag;

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void);
void EADC_FunctionTest(void);

/*---------------------------------------------------------------------------------------------------------*/
/* EADC interrupt handler                                                                                  */
/*---------------------------------------------------------------------------------------------------------*/
void EADC3_IRQHandler(void)
{
    if(EADC_GET_INT_FLAG(EADC, (0x1 << 4))) {
        g_u32AdcCmp0IntFlag = 1;
        EADC_CLR_INT_FLAG(EADC, (0x1 << 4));/* Clear the A/D compare flag 0 */
    }

    if(EADC_GET_INT_FLAG(EADC, (0x1 << 5))) {
        g_u32AdcCmp1IntFlag = 1;
        EADC_CLR_INT_FLAG(EADC, (0x1 << 5));/* Clear the A/D compare flag 1 */
    }
    EADC_CLR_INT_FLAG(EADC, (0x1 << 3));
}

void UART0_Init()
{
	/* Enable peripheral clock */
	CLK_EnableModuleClock(UART0_MODULE);

	/* Peripheral clock source */
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));

	 /* Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8) */
	SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk);
	SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD);

	/* Reset UART module */
	SYS_ResetModule(UART0_RST);

	/* Configure UART0 and set UART0 baud rate */
	UART_Open(UART0, 115200);
}

/*---------------------------------------------------------------------------------------------------------*/
/* EADC function test                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void EADC_FunctionTest()
{
    printf("\n");
    printf("+----------------------------------------------------------------------+\n");
    printf("|           EADC compare function (result monitor) sample code         |\n");
    printf("+----------------------------------------------------------------------+\n");
    printf("\nIn this test, software will compare the conversion result of channel 2.\n");
	
	// EADC Clock Initial.
	{
		/* Enable EADC module clock */
		CLK_EnableModuleClock(EADC_MODULE);

		/* EADC clock source is PCLK1 = 72MHz/2, set divider to 8, EADC clock is 36/8 MHz */
		CLK_SetModuleClock(EADC_MODULE, 0, CLK_CLKDIV0_EADC(8));

		/* Configure the GPA0 - GPA3 ADC analog input pins.  */
		SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA0MFP_Msk | SYS_GPA_MFPL_PA1MFP_Msk |
									 SYS_GPA_MFPL_PA2MFP_Msk | SYS_GPA_MFPL_PA3MFP_Msk);
		SYS->GPA_MFPL |= (SYS_GPA_MFPL_PA0MFP_EADC0_CH0 | SYS_GPA_MFPL_PA1MFP_EADC0_CH1 |
									SYS_GPA_MFPL_PA2MFP_EADC0_CH2 | SYS_GPA_MFPL_PA3MFP_EADC0_CH3);

		/* Disable the GPA0 - GPA3 digital input path to avoid the leakage current. */
		GPIO_DISABLE_DIGITAL_PATH(PA, 0xF);
	}
	
	/* Reset EADC module */
	SYS_ResetModule(EADC_RST);

	/* Set input mode as single-end and enable the A/D converter */
	EADC_Open(EADC, EADC_CTL_DIFFEN_SINGLE_END);

	/* Configure the sample module 0 for analog input channel 2 and ADINT0 trigger source */
	EADC_ConfigSampleModule(EADC, 0, EADC_ADINT0_TRIGGER, 2);
	/* Set sample module 0 external sampling time to 2 */
	EADC_SetExtendSampleTime(EADC, 0, 2);
	/* Enable EADC comparator 0. Compare condition: conversion result < 0x800; match Count=5 */
	printf("   Set the compare condition of comparator 0: channel 2 is less than 0x800; match count is 5.\n");
	EADC_ENABLE_CMP0(EADC, 0, EADC_CMP_CMPCOND_LESS_THAN, 0x800, 0x5);

	/* Enable EADC comparator 1. Compare condition: conversion result >= 0x800; match Count=5 */
	printf("   Set the compare condition of comparator 1 : channel 2 is greater than or equal to 0x800; match count is 5.\n");
	EADC_ENABLE_CMP1(EADC, 0, EADC_CMP_CMPCOND_GREATER_OR_EQUAL, 0x800, 0x5);

	/* Enable sample module 0 for ADINT0 */
	EADC_ENABLE_SAMPLE_MODULE_INT(EADC, 0, 0x1);
	/* Enable ADINT0 interrupt */
	EADC_ENABLE_INT(EADC, 0x1);

	/* Clear the A/D ADINT3 interrupt flag for safe */
	EADC_CLR_INT_FLAG(EADC, EADC_STATUS2_ADIF3_Msk);
	/* Enable sample module 0 for ADINT3 */
	EADC_ENABLE_SAMPLE_MODULE_INT(EADC, 3, 0x1);
	/* Enable ADINT3 interrupt */
	EADC_ENABLE_INT(EADC, (0x1 << 3));
	NVIC_EnableIRQ(EADC3_IRQn);

	/* Clear the EADC comparator 0 interrupt flag for safe */
	EADC_CLR_INT_FLAG(EADC, EADC_STATUS2_ADCMPF0_Msk);
	/* Enable ADC comparator 0 interrupt */
	EADC_ENABLE_CMP_INT(EADC, 0);

	/* Clear the EADC comparator 1 interrupt flag for safe */
	EADC_CLR_INT_FLAG(EADC, EADC_STATUS2_ADCMPF1_Msk);
	/* Enable ADC comparator 1 interrupt */
	EADC_ENABLE_CMP_INT(EADC, 1);

	/* Reset the EADC interrupt indicator and trigger sample module 0 to start A/D conversion */
	g_u32AdcCmp0IntFlag = 0;
	g_u32AdcCmp1IntFlag = 0;
	EADC_START_CONV(EADC, 0x1);

	/* Wait EADC compare interrupt */
	while((g_u32AdcCmp0IntFlag == 0) && (g_u32AdcCmp1IntFlag == 0));

	/* Disable the sample module 0 interrupt */
	EADC_DISABLE_SAMPLE_MODULE_INT(EADC, 0, 0x1);

	/* Disable ADC comparator interrupt */
	EADC_DISABLE_CMP_INT(EADC, 0);
	EADC_DISABLE_CMP_INT(EADC, 1);
	/* Disable compare function */
	EADC_DISABLE_CMP0(EADC);
	EADC_DISABLE_CMP1(EADC);

	if(g_u32AdcCmp0IntFlag == 1)
		printf("Comparator 0 interrupt occurs.\nThe conversion result of channel 2 is less than 0x800\n");
	else
		printf("Comparator 1 interrupt occurs.\nThe conversion result of channel 2 is greater than or equal to 0x800\n");
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

	/*---------------------------------------------------------------------------------------------------------*/
	/* SAMPLE CODE                                                                                             */
	/*---------------------------------------------------------------------------------------------------------*/

	printf("\nSystem clock rate: %d Hz", SystemCoreClock);

	/* EADC function test */
	EADC_FunctionTest();

	/* Disable EADC IP clock */
	CLK_DisableModuleClock(EADC_MODULE);

	/* Disable External Interrupt */
	NVIC_DisableIRQ(EADC3_IRQn);

	printf("Exit EADC sample code\n");

	while(1);
}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
