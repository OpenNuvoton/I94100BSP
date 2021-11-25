/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/03/14 10:04a $
 * @brief    Capture the PWM0 Channel 0 waveform by PWM0 Channel 2.
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "ConfigSysClk.h"
#include "Platform.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Macro, type and constant definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
void PWM_Init(void);
void UART0_Init(void);
void CalPeriodTime(PWM_T *PWM, uint32_t u32Ch);

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
	/* Init System, IP clock and multi-function I/O
		 In the end of SYS_Init() will issue SYS_LockReg()
		 to lock protected register. If user want to write
		 protected register, please issue SYS_UnlockReg()
		 to unlock protected register if necessary */

	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();
	
	/* Init UART to 115200-8n1 for print message */
	UART0_Init();

	// Init PWM0 Source Clock.
	PWM_Init();

	printf("\n\nCPU @ %dHz(PLL@ %dHz)\n", SystemCoreClock, PllClock);
	printf("PWM0 clock is from %s\n", (CLK->CLKSEL2 & CLK_CLKSEL2_PWM0SEL_Msk) ? "CPU" : "PLL");
	printf("+------------------------------------------------------------------------+\n");
	printf("|                          PWM Driver Sample Code                        |\n");
	printf("|                                                                        |\n");
	printf("+------------------------------------------------------------------------+\n");
	printf("  This sample code will use PWM0 channel 2 to capture\n  the signal from PWM0 channel 0.\n");
	printf("  I/O configuration:\n");
	printf("    PWM0 channel 2(PB.2) <--> PWM0 channel 0(PB.0)\n\n");
	printf("Use PWM0 Channel 2(PB.2) to capture the PWM0 Channel 0(PB.0) Waveform\n");

	while(1)
	{
		/*--------------------------------------------------------------------------------------*/
		/* Set the PWM0 Channel 0 as PWM output function.                                       */
		/*--------------------------------------------------------------------------------------*/

		/* Assume PWM output frequency is 250Hz and duty ratio is 30%, user can calculate PWM settings by follows.
			 duty ratio = (CMR+1)/(CNR+1)
			 cycle time = CNR+1
			 High level = CMR+1
			 PWM clock source frequency = PLL = 96000000
			 (CNR+1) = PWM clock source frequency/prescaler/PWM output frequency
							 = 96000000/6/250 = 64000
			 (Note: CNR is 16 bits, so if calculated value is larger than 65536, user should increase prescale value.)
			 CNR = 64000
			 duty ratio = 30% ==> (CMR+1)/(CNR+1) = 30%
			 CMR = 19200
			 Prescale value is 5 : prescaler= 6 */
		/* set PWM0 channel 0 output configuration */
		PWM_ConfigOutputChannel(PWM0, 0, 250, 30);
		/* Enable PWM Output path for PWM0 channel 0 */
		PWM_EnableOutput(PWM0, PWM_CH_0_MASK);
		
		/*--------------------------------------------------------------------------------------*/
		/* Set the PWM0 channel 2 for capture function                                          */
		/*--------------------------------------------------------------------------------------*/

		/* If input minimum frequency is 250Hz, user can calculate capture settings by follows.
			 Capture clock source frequency = PLL = 96000000 in the sample code.
			 (CNR+1) = Capture clock source frequency/prescaler/minimum input frequency
							 = 96000000/6/250 = 64000
			 (Note: CNR is 16 bits, so if calculated value is larger than 65536, user should increase prescale value.)
			 CNR = 0xFFFF
			 (Note: In capture mode, user should set CNR to 0xFFFF to increase capture frequency range.)

			 Capture unit time = 1/Capture clock source frequency/prescaler
			 62.5ns = 1/96000000/6 */
		/* set PWM0 channel 2 capture configuration */
		PWM_ConfigCaptureChannel(PWM0, 2, 62, 0);
		/* Enable falling capture reload */
		PWM_EnableFallingCaptureReload(PWM0, PWM_CH_2_MASK);
		/* Enable Capture Function for PWM0 channel 2 */
		PWM_EnableCapture(PWM0, PWM_CH_2_MASK);
		
		printf("\n\nPress any key to start PWM Capture Test\n");
		getchar();

		// Start PWM output and input.
		/* Enable Timer for PWM0 channel 0 */
		PWM_Start(PWM0, PWM_CH_0_MASK);
		/* Enable Timer for PWM0 channel 2 */
		PWM_Start(PWM0, PWM_CH_2_MASK);

		/* Wait until PWM0 channel 2 Timer start to count */
		while((PWM0->CNT[2]) == 0);

		/* Capture the Input Waveform Data */
		CalPeriodTime(PWM0, 2);
		
		/*---------------------------------------------------------------------------------------------------------*/
		/* Stop PWM0 channel 0 (Recommended procedure method 1)                                                    */
		/* Set PWM Timer loaded value(Period) as 0. When PWM internal counter(CNT) reaches to 0, disable PWM Timer */
		/*---------------------------------------------------------------------------------------------------------*/
		/* Set PWM0 channel 0 loaded value as 0 */
		PWM_Stop(PWM0, PWM_CH_0_MASK);

		/* Wait until PWM0 channel 0 Timer Stop */
		while(PWM_GET_COUNTER(PWM0, 0) != 0);
		
		/* Disable Timer for PWM0 channel 0 */
		PWM_ForceStop(PWM0, PWM_CH_0_MASK);

		/* Disable PWM Output path for PWM0 channel 0 */
		PWM_DisableOutput(PWM0, PWM_CH_0_MASK);

		/*---------------------------------------------------------------------------------------------------------*/
		/* Stop PWM0 channel 2 (Recommended procedure method 1)                                                    */
		/* Set PWM Timer loaded value(Period) as 0. When PWM internal counter(CNT) reaches to 0, disable PWM Timer */
		/*---------------------------------------------------------------------------------------------------------*/
		/* Set loaded value as 0 for PWM0 channel 2 */
		PWM_Stop(PWM0, PWM_CH_2_MASK);

		/* Wait until PWM0 channel 2 current counter reach to 0 */
		while(PWM_GET_COUNTER(PWM0, 2) != 0);

		/* Disable Timer for PWM0 channel 2 */
		PWM_ForceStop(PWM0, PWM_CH_2_MASK);

		/* Disable Capture Function and Capture Input path for  PWM0 channel 2*/
		PWM_DisableCapture(PWM0, PWM_CH_2_MASK);

		/* Clear Capture Interrupt flag for PWM0 channel 2 */
		PWM_ClearCaptureIntFlag(PWM0, 2, PWM_CAPTURE_INT_FALLING_LATCH);
	}
}

/*--------------------------------------------------------------------------------------*/
/* Capture function to calculate the input waveform information                         */
/* u32Count[4] : Keep the internal counter value when input signal rising / falling     */
/*               happens                                                                */
/*                                                                                      */
/* time    A    B     C     D                                                           */
/*           ___   ___   ___   ___   ___   ___   ___   ___                              */
/*      ____|   |_|   |_|   |_|   |_|   |_|   |_|   |_|   |_____                        */
/* index              0 1   2 3                                                         */
/*                                                                                      */
/* The capture internal counter down count from 0x10000, and reload to 0x10000 after    */
/* input signal falling happens (Time B/C/D)                                            */
/*--------------------------------------------------------------------------------------*/
void CalPeriodTime(PWM_T *PWM, uint32_t u32Ch)
{
	uint16_t u32Count[4];
	uint32_t u32i;
	uint16_t u16RisingTime, u16FallingTime, u16HighPeriod, u16LowPeriod, u16TotalPeriod;

	/* Clear Capture Falling Indicator (Time A) */
	PWM_ClearCaptureIntFlag(PWM, u32Ch, PWM_CAPTURE_INT_FALLING_LATCH);

	/* Wait for Capture Falling Indicator  */
	while(PWM_GetCaptureIntFlag(PWM, u32Ch) < 2);

	/* Clear Capture Falling Indicator (Time B)*/
	PWM_ClearCaptureIntFlag(PWM, u32Ch, PWM_CAPTURE_INT_FALLING_LATCH);

	u32i = 0;

	while(u32i < 4)
	{
		/* Wait for Capture Falling Indicator */
		while(PWM_GetCaptureIntFlag(PWM, u32Ch) < 2);

		/* Clear Capture Falling and Rising Indicator */
		PWM_ClearCaptureIntFlag(PWM, u32Ch, PWM_CAPTURE_INT_FALLING_LATCH | PWM_CAPTURE_INT_RISING_LATCH);

		/* Get Capture Falling Latch Counter Data */
		u32Count[u32i++] = PWM_GET_CAPTURE_FALLING_DATA(PWM, u32Ch);

		/* Wait for Capture Rising Indicator */
		while(PWM_GetCaptureIntFlag(PWM, u32Ch) < 2);

		/* Clear Capture Rising Indicator */
		PWM_ClearCaptureIntFlag(PWM, u32Ch, PWM_CAPTURE_INT_RISING_LATCH);

		/* Get Capture Rising Latch Counter Data */
		u32Count[u32i++] = PWM_GET_CAPTURE_RISING_DATA(PWM, u32Ch);
	}

	u16RisingTime = u32Count[1];

	u16FallingTime = u32Count[0];

	u16HighPeriod = u32Count[1] - u32Count[2];

	u16LowPeriod = 0x10000 - u32Count[1];

	u16TotalPeriod = 0x10000 - u32Count[2];

	printf("\nPWM generate: \nHigh Period=19198 ~ 19202, Low Period=44798 ~ 44802, Total Period=63999 ~ 64001\n");
	printf("\nCapture Result: Rising Time = %d, Falling Time = %d \nHigh Period = %d, Low Period = %d, Total Period = %d.\n\n",
				 u16RisingTime, u16FallingTime, u16HighPeriod, u16LowPeriod, u16TotalPeriod);
	if((u16HighPeriod < 19198) || (u16HighPeriod > 19202) || (u16LowPeriod < 44798) || (u16LowPeriod > 44802) || (u16TotalPeriod < 63999) || (u16TotalPeriod > 64001))
		printf("Capture Test Fail!!\n");
	else
		printf("Capture Test Pass!!\n");
}

/**
 * @brief       PWM0 IRQ Handler
 *
 * @param       None
 *
 * @return      None
 *
 * @details     ISR to handle PWM0 interrupt event
 */
void PWM0P1_IRQHandler(void)
{
	if(PWM_GetCaptureIntFlag(PWM0, 2) > 1)
	{
		PWM_ClearCaptureIntFlag(PWM0, 2, PWM_CAPTURE_INT_FALLING_LATCH);
	}
}

void PWM_Init(void)
{
	/*---------------------------------------------------------------------------------------------------------*/
	/* PWM clock frequency configuration                                                                       */
	/*---------------------------------------------------------------------------------------------------------*/
	/* PWM clock frequency can be set equal or double to HCLK by choosing case 1 or case 2 */
	/* case 1.PWM clock frequency is set equal to HCLK: select PWM module clock source as PCLK */
	CLK_SetModuleClock(PWM0_MODULE, CLK_CLKSEL2_PWM0SEL_PCLK0, NULL);
	
	/* Set PB multi-function pins for PWM0 Channel0~3 */
	SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB0MFP_Msk | SYS_GPB_MFPL_PB2MFP_Msk)) | (SYS_GPB_MFPL_PB0MFP_PWM0_CH0 | SYS_GPB_MFPL_PB2MFP_PWM0_CH2);
	
	/* Enable PWM0 module clock */
	CLK_EnableModuleClock(PWM0_MODULE);

	/* Reset PWM0 module */
	SYS_ResetModule(PWM0_RST);
}

void UART0_Init(void)
{
	/*---------------------------------------------------------------------------------------------------------*/
	/* Init UART                                                                                               */
	/*---------------------------------------------------------------------------------------------------------*/
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
