/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/03/09 10:05a $
 * @brief    Please refer readme.txt
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
volatile uint32_t g_u32WDTINTCounts;
volatile uint8_t g_u8IsWDTWakeupINT;

/**
 * @brief       IRQ Handler for WDT Interrupt
 *
 * @param       None
 *
 * @return      None
 *
 * @details     The WDT_IRQHandler is default IRQ of WDT, declared in startup .s.
 */
void WDT_IRQHandler(void)
{
    if(g_u32WDTINTCounts < 10)
        WDT_RESET_COUNTER();

    if(WDT_GET_TIMEOUT_INT_FLAG() == 1)
    {
        /* Clear WDT time-out interrupt flag */
        WDT_CLEAR_TIMEOUT_INT_FLAG();

        g_u32WDTINTCounts++;
    }

    if(WDT_GET_TIMEOUT_WAKEUP_FLAG() == 1)
    {
        /* Clear WDT time-out wake-up flag */
        WDT_CLEAR_TIMEOUT_WAKEUP_FLAG();

        g_u8IsWDTWakeupINT = 1;
    }
}
void UART0_Init(void)
{
    /* Enable peripheral clock */
    CLK_EnableModuleClock(UART0_MODULE);	
    /* Peripheral clock source */	
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
	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();
	
    /* Init UART0 for printf */
    UART0_Init();

    /* Set SysTick source to HCLK/2*/
    CLK_SetSysTickClockSrc(CLK_CLKSEL0_STCLKSEL_HCLK_DIV2);
    /* Enable LIRC */
    CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk);
    /* Waiting for clock ready */
    CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk);
    /* Enable peripheral clock */
    CLK_EnableModuleClock(WDT_MODULE);
    /* Peripheral clock source */
    CLK_SetModuleClock(WDT_MODULE, CLK_CLKSEL1_WDTSEL_LIRC, 0);

	
    printf("\n\nCPU @ %d Hz\n", SystemCoreClock);
    printf("+----------------------------------------+\n");
    printf("|    WDT Time-out Wake-up Sample Code    |\n");
    printf("+----------------------------------------+\n\n");

    /* To check if system has been reset by WDT time-out reset or not */
    if(WDT_GET_RESET_FLAG() == 1)
    {
        WDT_CLEAR_RESET_FLAG();
        printf("*** System has been reset by WDT time-out event ***\n\n");
        while(1);
    }

    printf("# WDT Settings:\n");
    printf("    - Clock source is LIRC                  \n");
    printf("    - Time-out interval is 2^14 * WDT clock \n");
    printf("      (around 1.6384 ~ 1.7408 s)            \n");
    printf("    - Interrupt enable                      \n");
    printf("    - Wake-up function enable               \n");
    printf("    - Reset function enable                 \n");
    printf("    - Reset delay period is 18 * WDT clock  \n");
    printf("# System will generate a WDT time-out interrupt event after 1.6384 ~ 1.7408 s, \n");
    printf("    and will be wake up from power-down mode also.\n");
    printf("    (Use PA.0 high/low period time to check WDT time-out interval)\n");
    printf("# When WDT interrupt counts large than 10, \n");
    printf("    we will not reset WDT counter value and system will be reset immediately by WDT time-out reset signal.\n");

    /* Use PA.0 to check time-out period time */
    PA->MODE = 0xFFFFFFFD;
    PA0 = 1;

    /* Enable WDT NVIC */
    NVIC_EnableIRQ(WDT_IRQn);

    /* Because of all bits can be written in WDT Control Register are write-protected;
       To program it needs to disable register protection first. */
    SYS_UnlockReg();

    /* Configure WDT settings and start WDT counting */
    g_u32WDTINTCounts = g_u8IsWDTWakeupINT = 0;
    WDT_Open(WDT_TIMEOUT_2POW14, WDT_RESET_DELAY_18CLK, TRUE, TRUE);

    /* Enable WDT interrupt function */
    WDT_EnableInt();

    while(1)
    {
        /* System enter to Power-down */
        /* To program PWRCTL register, it needs to disable register protection first. */
        SYS_UnlockReg();
        printf("\nSystem enter to power-down mode ...\n");
        /* To check if all the debug messages are finished */
        while(IsDebugFifoEmpty() == 0);
        CLK_PowerDown();

        /* Check if WDT time-out interrupt and wake-up occurred or not */
        while(g_u8IsWDTWakeupINT == 0);

        g_u8IsWDTWakeupINT = 0;
        PA0 ^= 1;

        printf("System has been waken up done. WDT interrupt counts: %d.\n\n", g_u32WDTINTCounts);
    }
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
