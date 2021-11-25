/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/03/14 10:04a $
 * @brief
 *           Change duty cycle and period of output waveform by PWM Double Buffer function.
 *
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "Platform.h"
#include "ConfigSysClk.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Macro, type and constant definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/

/**
 * @brief       PWM0 IRQ Handler
 *
 * @param       None
 *
 * @return      None
 *
 * @details     ISR to handle PWM0 interrupt event
 */
void PWM0P0_IRQHandler(void)
{
    static int toggle = 0;

    // Update PWM0 channel 0 period and duty
    if(toggle == 0)
    {
        PWM_SET_CNR(PWM0, 0, 99);
        PWM_SET_CMR(PWM0, 0, 39);
    }
    else
    {
        PWM_SET_CNR(PWM0, 0, 399);
        PWM_SET_CMR(PWM0, 0, 199);
    }
    toggle ^= 1;
    // Clear channel 0 period interrupt flag
    PWM_ClearPeriodIntFlag(PWM0, 0);
}

void UART0_Init()
{
	    /* Enable UART module clock */
    CLK_EnableModuleClock(UART0_MODULE);

    /* Select UART module clock source as PLL and UART module clock divider as 1 */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));
	
/*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8) */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk)) | (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD);
	
    /* Reset UART module */
    SYS_ResetModule(UART0_RST);

    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
}

void PWM_Init()
{
	/* Enable PWM0 module clock */
	CLK_EnableModuleClock(PWM0_MODULE);

	/*---------------------------------------------------------------------------------------------------------*/
	/* PWM clock frequency configuration                                                                       */
	/*---------------------------------------------------------------------------------------------------------*/
	/* PWM clock frequency can be set equal or double to HCLK by choosing case 1 or case 2 */
	/* case 1.PWM clock frequency is set equal to HCLK: select PWM module clock source as PCLK */
	CLK_SetModuleClock(PWM0_MODULE, CLK_CLKSEL2_PWM0SEL_PCLK0, NULL);

	/* case 2.PWM clock frequency is set double to HCLK: select PWM module clock source as PLL */
	//CLK_SetModuleClock(PWM0_MODULE, CLK_CLKSEL2_PWM0SEL_PLL, NULL);
	/*---------------------------------------------------------------------------------------------------------*/

	/* Reset PWM0 module */
	SYS_ResetModule(PWM0_RST);

	/* Set PB multi-function pins for PWM0 Channel0~3 */
	SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB4MFP_Msk | SYS_GPB_MFPL_PB5MFP_Msk | SYS_GPB_MFPL_PB6MFP_Msk | SYS_GPB_MFPL_PB7MFP_Msk)) | (SYS_GPB_MFPL_PB4MFP_PWM0_CH0 | SYS_GPB_MFPL_PB5MFP_PWM0_CH1 | SYS_GPB_MFPL_PB6MFP_PWM0_CH2 | SYS_GPB_MFPL_PB7MFP_PWM0_CH3);
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

	// PWM Initial
	PWM_Init();

	printf("\n\nCPU @ %dHz(PLL@ %dHz)\n", SystemCoreClock, PllClock);
	printf("PWM0 clock is from %s\n", (CLK->CLKSEL2 & CLK_CLKSEL2_PWM0SEL_Msk) ? "CPU" : "PLL");
	printf("+------------------------------------------------------------------------+\n");
	printf("|                          PWM Driver Sample Code                        |\n");
	printf("|                                                                        |\n");
	printf("+------------------------------------------------------------------------+\n");
	printf("  This sample code will use PWM0 channel 0 to output waveform\n");
	printf("  I/O configuration:\n");
	printf("    waveform output pin: PWM0 channel 0(PB.4)\n");
	printf("\nUse double buffer feature.\n");

	/*
			PWM0 channel 0 waveform of this sample shown below:

			|<-        CNR + 1  clk     ->|  CNR + 1 = 399 + 1 CLKs
										 |<-CMR+1 clk ->|  CMR + 1 = 199 + 1 CLKs
																		|<-   CNR + 1  ->|  CNR + 1 = 99 + 1 CLKs
																						 |<CMR+1>|  CMR + 1 = 39 + 1 CLKs
		__                ______________          _______
			|______200_____|     200      |____60__|   40  |_____PWM waveform

	*/


	/*
		Configure PWM0 channel 0 init period and duty.
		Period is PLL / (prescaler * (CNR + 1))
		Duty ratio = (CMR + 1) / (CNR + 1)
		Period = 72 MHz / (2 * (199 + 1)) = 180000 Hz
		Duty ratio = (99 + 1) / (199 + 1) = 50%
	*/
	// PWM0 channel 0 frequency is 180000Hz, duty 50%,
	PWM_ConfigOutputChannel(PWM0, 0, 180000, 50);

	// Enable output of PWM0 channel 0
	PWM_EnableOutput(PWM0, PWM_CH_0_MASK);

	// Enable PWM0 channel 0 period interrupt, use channel 0 to measure time.
	PWM_EnablePeriodInt(PWM0, 0, 0);
	NVIC_EnableIRQ(PWM0_P0_IRQn);

	// Start
	PWM_Start(PWM0, PWM_CH_0_MASK);

	while(1);
}
