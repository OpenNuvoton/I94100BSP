/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/03/09 10:04a $
 * @brief    Implement timer1 event counter function to count the external input event.
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

/*---------------------------------------------------------------------------------------------------------*/
/*  Generate Event Counter Source by specify GPIO pin                                                      */
/*---------------------------------------------------------------------------------------------------------*/
void GenerateEventCounterSource(uint32_t u32Port, uint32_t u32Pin, uint32_t u32Counts)
{
    while(u32Counts--)
    {
			GPIO_PIN_DATA(u32Port, u32Pin) = 1;
			
			while((GPIO_GET_IN_DATA(PD) & (BIT0 << u32Pin)) == 0)
			{
			}
			
			GPIO_PIN_DATA(u32Port, u32Pin) = 0;
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

	  SYSCLK_INITIATE();
	
    /* Enable peripheral clock */
    CLK_EnableModuleClock(TMR2_MODULE);

    /* Peripheral clock source */
    CLK_SetModuleClock(TMR2_MODULE, CLK_CLKSEL1_TMR2SEL_PCLK1, 0);

	  /* Timer2 event counter pin */
	  SYS->GPB_MFPL = (SYS->GPB_MFPL & ~SYS_GPB_MFPL_PB2MFP_Msk) | SYS_GPB_MFPL_PB2MFP_T2;
    /* Init UART0 for printf */
    UART0_Init();

    printf("\n\nCPU @ %d Hz\n", SystemCoreClock);
    printf("+----------------------------------------------+\n");
    printf("|    Timer2 Event Counter Input Sample Code    |\n");
    printf("+----------------------------------------------+\n\n");

    printf("# Timer2 Settings:\n");
    printf("    - Clock source is HCLK      \n");
    printf("    - Continuous counting mode  \n");
    printf("    - Interrupt enable          \n");
    printf("    - Event counter mode enable \n");
    printf("    - Compared value is 56789   \n");
    printf("# Connect PD.4 pin to event counter pin T2(PB.2) and pull PD.4 High/Low to generate T2 event input source.\n\n");

    /* Configure PD.4 as GPIO output pin and pull initial pin status to Low */
    PD->MODE = 0xFFFFFDFF;
    PD4 = 0;

    /* Enable Timer2 NVIC */
    NVIC_EnableIRQ(TMR2_IRQn);

    /* Clear Timer2 interrupt counts to 0 */
    g_au32TMRINTCount[2] = 0;

    /* Configure Timer2 settings and for event counter application */
    TIMER_Open(TIMER2, TIMER_CONTINUOUS_MODE, 1);
    TIMER_SET_PRESCALE_VALUE(TIMER2, 0);
    TIMER_SET_CMP_VALUE(TIMER2, 56789);
    TIMER_EnableEventCounter(TIMER2, TIMER_COUNTER_FALLING_EDGE);
    TIMER_EnableInt(TIMER2);

    /* Start Timer2 counting */
    TIMER_Start(TIMER2);

    /* To check if counter value of Timer2 should be 0 while event counter mode is enabled */
    if(TIMER_GetCounter(TIMER2) != 0)
    {
        printf("Default counter value is not 0. (%d)\n", TIMER_GetCounter(TIMER2));

        /* Stop Timer2 counting */
        TIMER_Close(TIMER2);
        while(1);
    }

    printf("Start to check Timer2 counter value ......\n\n");

    /* To generate one counter event from PD.4 to T2 pin */
    GenerateEventCounterSource(3, 4, 1);

    /* To check if counter value of Timer2 should be 1 */
    while(TIMER_GetCounter(TIMER2) == 0);
    if(TIMER_GetCounter(TIMER2) != 1)
    {
        printf("Get unexpected counter value. (%d)\n", TIMER_GetCounter(TIMER2));

        /* Stop Timer2 counting */
        TIMER_Close(TIMER2);
        while(1);
    }

    /* To generate remains counts to T2 pin */
    GenerateEventCounterSource(3, 4, (56789 - 1));

    while(1)
    {
        if(g_au32TMRINTCount[2] == 1)
        {
            printf("# Timer2 interrupt event occurred.\n");
            break;
        }
    }

    printf("# Get Timer2 event counter value is %d .... ", TIMER_GetCounter(TIMER2));
    if(TIMER_GetCounter(TIMER2) == 56789)
    {
        printf("PASS.\n");
    }
    else
    {
        printf("FAIL.\n");
    }

    /* Stop Timer2 counting */
    TIMER_Close(TIMER2);

    while(1);
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
