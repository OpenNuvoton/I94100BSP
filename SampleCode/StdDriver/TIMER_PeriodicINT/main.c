/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/03/09 10:04a $
 * @brief    Implement timer counting in periodic mode.
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
}

/**
 * @brief       Timer1 IRQ
 * @param       None
 * @return      None
 * @details     The Timer1 default IRQ, declared in startup .s.
 */
void TMR1_IRQHandler(void)
{
    if(TIMER_GetIntFlag(TIMER1) == 1)
    {
        /* Clear Timer1 time-out interrupt flag */
        TIMER_ClearIntFlag(TIMER1);

        g_au32TMRINTCount[1]++;
    }
}

/**
 * @brief       Timer2 IRQ
 * @param       None
 * @return      None
 * @details     The Timer2 default IRQ, declared in startup .s.
 */
void TMR2_IRQHandler(void)
{
    if(TIMER_GetIntFlag(TIMER2) == 1)
    {
        /* Clear Timer2 time-out interrupt flag */
        TIMER_ClearIntFlag(TIMER2);

        g_au32TMRINTCount[2]++;
    }
}

/**
 * @brief       Timer3 IRQ
 * @param       None
 * @return      None
 * @details     The Timer3 default IRQ, declared in startup .s.
 */
void TMR3_IRQHandler(void)
{
    if(TIMER_GetIntFlag(TIMER3) == 1)
    {
        /* Clear Timer3 time-out interrupt flag */
        TIMER_ClearIntFlag(TIMER3);

        g_au32TMRINTCount[3]++;
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
	uint8_t u8Lock;

	SYSCLK_INITIATE();
	
	/* Enable peripheral clock */
	CLK_EnableModuleClock(TMR0_MODULE);
	CLK_EnableModuleClock(TMR1_MODULE);
	CLK_EnableModuleClock(TMR2_MODULE);
	CLK_EnableModuleClock(TMR3_MODULE);

	// Enable HXT clock for TIMER0 and TIMER3.
	u8Lock = SYS_Unlock();
	CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);
	CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);
	SYS_Lock(u8Lock);

	/* Peripheral clock source */
	CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HXT, 0);
	CLK_SetModuleClock(TMR1_MODULE, CLK_CLKSEL1_TMR1SEL_PCLK0, 0);
	CLK_SetModuleClock(TMR2_MODULE, CLK_CLKSEL1_TMR2SEL_HIRC, 0);
	CLK_SetModuleClock(TMR3_MODULE, CLK_CLKSEL1_TMR3SEL_HXT, 0);
		
	/* Init UART0 for printf */
	UART0_Init();

	printf("\n\nCPU @ %d Hz\n", SystemCoreClock);
	printf("+--------------------------------------------+\n");
	printf("|    Timer Periodic Interrupt Sample Code    |\n");
	printf("+--------------------------------------------+\n\n");

	printf("# Timer0 Settings:\n");
	printf("    - Clock source is HXT       \n");
	printf("    - Time-out frequency is 1 Hz\n");
	printf("    - Periodic mode             \n");
	printf("    - Interrupt enable          \n");
	printf("# Timer1 Settings:\n");
	printf("    - Clock source is HCLK      \n");
	printf("    - Time-out frequency is 2 Hz\n");
	printf("    - Periodic mode             \n");
	printf("    - Interrupt enable          \n");
	printf("# Timer2 Settings:\n");
	printf("    - Clock source is HIRC      \n");
	printf("    - Time-out frequency is 4 Hz\n");
	printf("    - Periodic mode             \n");
	printf("    - Interrupt enable          \n");
	printf("# Timer3 Settings:\n");
	printf("    - Clock source is HXT       \n");
	printf("    - Time-out frequency is 8 Hz\n");
	printf("    - Periodic mode             \n");
	printf("    - Interrupt enable          \n");
	printf("# Check Timer0 ~ Timer3 interrupt counts are reasonable or not.\n\n");

	/* Open Timer0 in periodic mode, enable interrupt and 1 interrupt tick per second */
	TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 1);
	TIMER_EnableInt(TIMER0);

	/* Open Timer1 in periodic mode, enable interrupt and 2 interrupt ticks per second */
	TIMER_Open(TIMER1, TIMER_PERIODIC_MODE, 2);
	TIMER_EnableInt(TIMER1);

	/* Open Timer2 in periodic mode, enable interrupt and 4 interrupt ticks per second */
	TIMER_Open(TIMER2, TIMER_PERIODIC_MODE, 4);
	TIMER_EnableInt(TIMER2);

	/* Open Timer3 in periodic mode, enable interrupt and 8 interrupt ticks per second */
	TIMER_Open(TIMER3, TIMER_PERIODIC_MODE, 8);
	TIMER_EnableInt(TIMER3);

	/* Enable Timer0 ~ Timer3 NVIC */
	NVIC_EnableIRQ(TMR0_IRQn);
	NVIC_EnableIRQ(TMR1_IRQn);
	NVIC_EnableIRQ(TMR2_IRQn);
	NVIC_EnableIRQ(TMR3_IRQn);

	/* Clear Timer0 ~ Timer3 interrupt counts to 0 */
	g_au32TMRINTCount[0] = g_au32TMRINTCount[1] = g_au32TMRINTCount[2] = g_au32TMRINTCount[3] = 0;
	u32InitCount = g_au32TMRINTCount[0];

	/* Start Timer0 ~ Timer3 counting */
	TIMER_Start(TIMER0);
	TIMER_Start(TIMER1);
	TIMER_Start(TIMER2);
	TIMER_Start(TIMER3);

	/* Check Timer0 ~ Timer3 interrupt counts */
	printf("# Timer interrupt counts :\n");
	while(u32InitCount < 20)
	{
		uint32_t u32Temp;
		
		u32Temp = u32InitCount;
		if(g_au32TMRINTCount[0] != u32Temp)
		{
			printf("    TMR0:%3d    TMR1:%3d    TMR2:%3d    TMR3:%3d\n",
						 g_au32TMRINTCount[0], g_au32TMRINTCount[1], g_au32TMRINTCount[2], g_au32TMRINTCount[3]);
			u32InitCount = g_au32TMRINTCount[0];
		}
	}

	printf("*** End ***\n");

	while(1);
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
