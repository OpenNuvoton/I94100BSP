/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/03/14 10:04a $
 * @brief    Show how to use the timer2 capture function to capture timer2 counter value.
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "Platform.h"
#include "ConfigSysClk.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Global Interface Variables Declarations                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
volatile uint32_t g_au32TMRINTCount[4] = {0};


/**
  * @brief      Timer2 IRQ
  *
  * @param      None
  *
  * @return     None
  *
  * @details    The Timer2 default IRQ, declared in startup_M451Series.s.
  */
void TMR2_IRQHandler(void)
{
    if(TIMER_GetCaptureIntFlag(TIMER2) == 1)
    {
        /* Clear Timer2 capture trigger interrupt flag */
        TIMER_ClearCaptureIntFlag(TIMER2);

        g_au32TMRINTCount[2]++;
    }
}

void UART0_Init(void)
{
	CLK_EnableModuleClock(UART0_MODULE);
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));
	/* Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8) */
	SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk)) | (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD);
    
	/* Reset UART module */
	SYS_ResetModule(UART0_RST);
	/* Configure UART0 and set UART0 Baudrate */
	UART_Open(UART0, 115200);
}

/*---------------------------------------------------------------------------------------------------------*/
/*  MAIN function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int main(void)
{
	volatile uint32_t u32InitCount;
	uint32_t au32CAPValue[10], u32CAPDiff;

	SYSCLK_INITIATE();

	/* Enable peripheral clock */
	CLK_EnableModuleClock(TMR0_MODULE);
	CLK_EnableModuleClock(TMR2_MODULE);
	CLK_EnableModuleClock(TMR3_MODULE);

	/* Peripheral clock source */
	CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_PCLK0, 0);
	CLK_SetModuleClock(TMR2_MODULE, CLK_CLKSEL1_TMR2SEL_PCLK1, 0);
	CLK_SetModuleClock(TMR3_MODULE, CLK_CLKSEL1_TMR3SEL_PCLK1, 0);

	SYS->GPA_MFPH = (SYS->GPA_MFPH & ~SYS_GPA_MFPH_PA14MFP_Msk) | SYS_GPA_MFPH_PA14MFP_T1;
	SYS->GPB_MFPL = (SYS->GPB_MFPL & ~(SYS_GPB_MFPL_PB3MFP_Msk | SYS_GPB_MFPL_PB2MFP_Msk)) | (SYS_GPB_MFPL_PB3MFP_T2_EXT | SYS_GPB_MFPL_PB2MFP_T2);
	SYS->GPC_MFPL = (SYS->GPC_MFPL & ~SYS_GPC_MFPL_PC2MFP_Msk) | SYS_GPC_MFPL_PC2MFP_T3;
	SYS->GPD_MFPL = (SYS->GPD_MFPL & ~SYS_GPD_MFPL_PD2MFP_Msk) | SYS_GPD_MFPL_PD2MFP_TIM0;

	/* Init UART0 for printf */
	UART0_Init();

	printf("\n\nCPU @ %d Hz\n", SystemCoreClock);
	printf("+------------------------------------------+\n");
	printf("|    Timer2 Capture Counter Sample Code    |\n");
	printf("+------------------------------------------+\n\n");

	printf("# Timer0 Settings:\n");
	printf("    - Clock source is HXT\n");
	printf("    - Time-out frequency is 1000 Hz    			\n");
	printf("    - Toggle-output mode and frequency is 500 Hz\n");
	printf("# Timer3 Settings:\n");
	printf("    - Clock source is HXT\n");
	printf("    - Time-out frequency is 2 Hz    			\n");
	printf("    - Toggle-output mode and frequency is 1 Hz	\n");
	printf("# Timer2 Settings:\n");
	printf("    - Clock source is HCLK              \n");
	printf("    - Continuous counting mode          \n");
	printf("    - Interrupt enable                  \n");
	printf("    - Compared value is 0xFFFFFF        \n");
	printf("    - Event counter mode enable         \n");
	printf("    - External capture mode enable      \n");
	printf("    - Capture trigger interrupt enable  \n");
	printf("# Connect T0(PD.2) toggle-output pin to T2(PB.2) event counter pin.\n");
	printf("# Connect T3(PC.2) toggle-output pin to T2_EXT(PB.3) external capture pin.\n\n");

	printf("# Every 500 event counts will be captured when Time2 capture trigger event occurred.\n");

	/* Enable Timer2 NVIC */
	NVIC_EnableIRQ(TMR2_IRQn);

	/* Open Timer0 in toggle-output mode and toggle-output frequency is 500 Hz*/
	TIMER_Open(TIMER0, TIMER_TOGGLE_MODE, 1000);

	/* Open Timer3 in toggle-output mode and toggle-output frequency is 1 Hz */
	TIMER_Open(TIMER3, TIMER_TOGGLE_MODE, 2);

	/* Clear Timer2 interrupt counts to 0 */
	u32InitCount = g_au32TMRINTCount[2] = 0;

	/* Enable Timer2 event counter input and external capture function */
	TIMER_Open(TIMER2, TIMER_CONTINUOUS_MODE, 1);
	TIMER_SET_PRESCALE_VALUE(TIMER2, 0);
	TIMER_SET_CMP_VALUE(TIMER2, 0xFFFFFF);
	TIMER_EnableEventCounter(TIMER2, TIMER_COUNTER_FALLING_EDGE);
	TIMER2->EXTCTL &= ~(TMR_EXTCTL_ECNTSSEL_Msk);
	TIMER_EnableCapture(TIMER2, TIMER_CAPTURE_FREE_COUNTING_MODE, TIMER_CAPTURE_FALLING_EDGE);
	TIMER_EnableInt(TIMER2);
	TIMER_EnableCaptureInt(TIMER2);

	/* Start Timer0, Timer3 and Timer2 counting */
	TIMER_Start(TIMER0);
	TIMER_Start(TIMER3);
	TIMER_Start(TIMER2);

	/* Check Timer2 capture trigger interrupt counts */
	while(g_au32TMRINTCount[2] <= 10)
	{
		uint32_t u32Temp;
		
		u32Temp = u32InitCount;
		if(g_au32TMRINTCount[2] != u32Temp)
		{
			au32CAPValue[u32InitCount] = TIMER_GetCaptureData(TIMER2);
			
			u32Temp = au32CAPValue[u32InitCount];
			u32CAPDiff = u32Temp - au32CAPValue[u32InitCount - 1];
			printf("    [%2d]: %4d. Diff: %d.\n", g_au32TMRINTCount[2], u32Temp, u32CAPDiff);
			if(u32InitCount > 1)
			{
				if(u32CAPDiff != 500)
				{
					printf("*** FAIL ***\n");
					while(1);
				}
			}
			u32InitCount = g_au32TMRINTCount[2];
		}
		
		if(u32InitCount >= 10)
			break;
	}

	/* Stop Timer0, Timer2 and Timer3 counting */
	TIMER0->CTL = 0;
	TIMER2->CTL = 0;
	TIMER3->CTL = 0;

	printf("*** PASS ***\n");

	while(1);
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
