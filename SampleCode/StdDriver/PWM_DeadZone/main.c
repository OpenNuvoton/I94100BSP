/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/03/14 10:04a $
 * @brief    Demonstrate how to use PWM Dead Zone function.
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include "ConfigSysClk.h"
#include "Platform.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Macro, type and constant definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/

void UART0_Init()
{
    /* Enable UART module clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Select UART module clock source as PLL and UART module clock divider as 1 */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));
	
    /* Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8) */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk)) | (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD);
	
    /* Reset UART module */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
}

void PWM_Init()
{
	/*---------------------------------------------------------------------------------------------------------*/
	/* PWM clock frequency configuration                                                                       */
	/*---------------------------------------------------------------------------------------------------------*/
	/* PWM clock frequency can be set equal or double to HCLK by choosing case 1 or case 2 */
	/* case 1.PWM clock frequency is set equal to HCLK: select PWM module clock source as PCLK */
	CLK_SetModuleClock(PWM0_MODULE, CLK_CLKSEL2_PWM0SEL_PCLK0, NULL);

	/* case 2.PWM clock frequency is set double to HCLK: select PWM module clock source as PLL */
	//CLK_SetModuleClock(PWM0_MODULE, CLK_CLKSEL2_PWM0SEL_PLL, NULL);
	
	/* Enable PWM0 module clock */
	CLK_EnableModuleClock(PWM0_MODULE);
	/*---------------------------------------------------------------------------------------------------------*/

	/* Reset PWM0 module */
	SYS_ResetModule(PWM0_RST);

	/* Set PB multi-function pins for PWM0 Channel0~3 */
	SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB0MFP_Msk | SYS_GPB_MFPL_PB1MFP_Msk | SYS_GPB_MFPL_PB2MFP_Msk | SYS_GPB_MFPL_PB3MFP_Msk)) | (SYS_GPB_MFPL_PB0MFP_PWM0_CH0 | SYS_GPB_MFPL_PB1MFP_PWM0_CH1 | SYS_GPB_MFPL_PB2MFP_PWM0_CH2 | SYS_GPB_MFPL_PB3MFP_PWM0_CH3);

}
/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	/* Init UART to 115200-8n1 for print message */
	UART0_Init();

	// PWM Initial.
	PWM_Init();

	printf("\n\nCPU @ %dHz(PLL@ %dHz)\n", SystemCoreClock, PllClock);
	printf("PWM0 clock is from %s\n", (CLK->CLKSEL2 & CLK_CLKSEL2_PWM0SEL_Msk) ? "CPU" : "PLL");
	printf("+------------------------------------------------------------------------+\n");
	printf("|                          PWM Driver Sample Code                        |\n");
	printf("|                                                                        |\n");
	printf("+------------------------------------------------------------------------+\n");
	printf("  This sample code will output PWM signals between channel 0~3 with two kinds of different\n");
	printf("  frequency and duty, and enable dead zone function on channel 0 and 2 pairs.\n");
	printf("  I/O configuration:\n");
	printf("    waveform output pin: PWM0_CH0(PB.0), PWM0_CH1(PB.1), PWM0_CH2(PB.2) and PWM0_CH3(PB.3)\n");

	/* Set PWM0 channel 0 and 2 as complementary mode */
	PWM_ENABLE_COMPLEMENTARY_CHAN_MODE(PWM0, PWM_CHAN0_COMP_OUTPUT | PWM_CHAN2_COMP_OUTPUT);

	// PWM0 channel 0 frequency is 100Hz, duty 30%,
	PWM_ConfigOutputChannel(PWM0, 0, 100, 30);
	SYS_UnlockReg();
	PWM_EnableDeadZone(PWM0, 0, 400);
	SYS_LockReg();

	// PWM0 channel 2 frequency is 300Hz, duty 50%
	PWM_ConfigOutputChannel(PWM0, 2, 300, 50);
	SYS_UnlockReg();
	PWM_EnableDeadZone(PWM0, 2, 200);
	SYS_LockReg();

	// Enable output of PWM0 channel 0~3
	PWM_EnableOutput(PWM0, 0xF);

	// Start
	PWM_Start(PWM0, 0xF);

	while(1);
}
