/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * $Revision: 1 $
 * $Date: 17/03/10 10:04a $
 * @brief
 *           Use timer0 periodic time-out interrupt event to wake up system.
 *
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "Platform.h"
#include "ConfigSysClk.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Global Interface Variables Declarations                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
extern int IsDebugFifoEmpty(void);
volatile uint8_t g_u8IsTMR0WakeupFlag = 0;
volatile uint32_t g_au32TMRINTCount[4] = {0};

/**
 * @brief       Timer0 IRQ
 * @param       None
 * @return      None
 * @details     The Timer0 default IRQ, declared in startup .s.
 */
void TMR0_IRQHandler(void)
{
    if(TIMER_GetIntFlag(TIMER0) == 1)
    {
        /* Clear Timer0 time-out interrupt flag */
        TIMER_ClearIntFlag(TIMER0);

        g_au32TMRINTCount[0]++;
    }

    if(TIMER_GetWakeupFlag(TIMER0) == 1)
    {
        /* Clear Timer0 wake-up flag */
        TIMER_ClearWakeupFlag(TIMER0);

        g_u8IsTMR0WakeupFlag = 1;
    }
}

void UART0_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init UART                                                                                               */
    /*---------------------------------------------------------------------------------------------------------*/
	 CLK_EnableModuleClock(UART0_MODULE);
	 CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));
	/* Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8) */
    SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk)) | (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD);
    /* Reset IP */
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
	uint8_t u8Lock;


	SYSCLK_INITIATE();

	// Enable LIRC clock for TIMER0.
	u8Lock = SYS_Unlock();
	CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk);
	CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk);
	SYS_Lock(u8Lock);

	/* Enable peripheral clock */
	CLK_EnableModuleClock(TMR0_MODULE);

	/* Peripheral clock source */
	CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_LIRC, 0);
    
	/* Init UART0 for printf */
	UART0_Init();

	printf("\n\nCPU @ %d Hz\n", SystemCoreClock);
	printf("+-------------------------------------------+\n");
	printf("|    Timer0 Time-out Wake-up Sample Code    |\n");
	printf("+-------------------------------------------+\n\n");

	printf("# Timer0 Settings:\n");
	printf("    - Clock source is LIRC          \n");
	printf("    - Time-out frequency is 1 Hz    \n");
	printf("    - Periodic mode                 \n");
	printf("    - Interrupt enable              \n");
	printf("    - Wake-up function enable       \n");
	printf("# System will enter to Power-down mode while Timer0 interrupt counts is reaching 3.\n");
	printf("  And will be wakeup while Timer0 interrupt counts is reaching 4.\n\n");

	/* Enable Timer0 NVIC */
	NVIC_EnableIRQ(TMR0_IRQn);

	/* Open Timer0 time-out frequency to 1 Hz in periodic mode */
	TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 1);

	/* Enable Timer0 time-out interrupt and wake-up function */
	TIMER_EnableInt(TIMER0);
	TIMER_EnableWakeup(TIMER0);

	/* Start Timer0 counting */
	TIMER_Start(TIMER0);

	u32InitCount = g_u8IsTMR0WakeupFlag = g_au32TMRINTCount[0] = 0;
	while(g_au32TMRINTCount[0] < 10)
	{
		uint32_t u32Temp;
		
		u32Temp = u32InitCount;
		if(g_au32TMRINTCount[0] != u32Temp)
		{
			printf("Timer0 interrupt counts - %d\n", g_au32TMRINTCount[0]);
			if(g_au32TMRINTCount[0] == 3)
			{
				/* System enter to Power-down */
				/* To program PWRCTL register, it needs to disable register protection first. */
				SYS_UnlockReg();
				printf("\nSystem enter to power-down mode ...\n");
				/* To check if all the debug messages are finished */
				while(IsDebugFifoEmpty() == 0);
				CLK_PowerDown();

				/* Check if Timer0 time-out interrupt and wake-up flag occurred */
				while(g_u8IsTMR0WakeupFlag == 0);

				printf("System has been waken-up done. (Timer0 interrupt counts is %d)\n\n", g_au32TMRINTCount[0]);
			}
			u32InitCount = g_au32TMRINTCount[0];
		}
	}

	/* Stop Timer0 counting */
	TIMER_Stop(TIMER0);

	printf("*** PASS ***\n");

	while(1);
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
