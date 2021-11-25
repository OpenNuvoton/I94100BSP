/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/03/09 10:04a $
 * @brief    Show how to use timer0 to create various delay time.
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "Platform.h"
#include "ConfigSysClk.h"

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
    volatile uint32_t u32DelayTime;
		uint32_t u32Temp;
	
    SYSCLK_INITIATE();
	
	  /* Enable peripheral clock */
    CLK_EnableModuleClock(TMR0_MODULE);
    CLK_EnableModuleClock(TMR1_MODULE);
	
    /* Peripheral clock source */
    CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_PCLK0, 0);
    CLK_SetModuleClock(TMR1_MODULE, CLK_CLKSEL1_TMR1SEL_HIRC, 0);
	
    /* Init UART0 for printf */
    UART0_Init();

    printf("\n\nCPU @ %d Hz\n", SystemCoreClock);
    printf("+-----------------------------------+\n");
    printf("|    Timer Delay API Sample Code    |\n");
    printf("+-----------------------------------+\n\n");

    printf("# This sample code is using Timer1 to check Timer0 TIMER_Delay API delay time is reasonable or not.\n");
    printf("# Delay time includes 100 ms, 200 ms, 300 ms, 400 ms and 500 ms.\n\n");

    /* Start Timer1 to measure delay period of TIMER_Delay API is reasonable or not */
		// Open Timer to set operation mode.
		TIMER_Open(TIMER1, TIMER_PERIODIC_MODE, 1000);
		
		// Set prescale value in order TIMER1 counter cycle is 1ms.
		u32Temp = TIMER_GetModuleClock(TIMER1);
		u32Temp = u32Temp/1000000; // divide by 1000000Hz(1 micro second frequency).
		u32Temp = u32Temp - 1; // adjust for register value.
    TIMER_SET_PRESCALE_VALUE(TIMER1, u32Temp);
		
		TIMER_SET_CMP_VALUE(TIMER1, 0xFFFFFF);
    TIMER_Start(TIMER1);

    TIMER_SET_CMP_VALUE(TIMER1, 0xFFFFFF);
    TIMER_Delay(TIMER0, 100000);
    u32DelayTime = TIMER_GetCounter(TIMER1) / 1000;
    printf("    Check DelayTime-1 is %d ms .... ", u32DelayTime);
    if(u32DelayTime == 100)
        printf("PASS.\n");
    else
        printf("FAIL.\n");

    TIMER_SET_CMP_VALUE(TIMER1, 0xFFFFFF);
    TIMER_Delay(TIMER0, 200000);
    u32DelayTime = TIMER_GetCounter(TIMER1) / 1000;
    printf("    Check DelayTime-2 is %d ms .... ", u32DelayTime);
    if(u32DelayTime == 200)
        printf("PASS.\n");
    else
        printf("FAIL.\n");

    TIMER_SET_CMP_VALUE(TIMER1, 0xFFFFFF);
    TIMER_Delay(TIMER0, 300000);
    u32DelayTime = TIMER_GetCounter(TIMER1) / 1000;
    printf("    Check DelayTime-3 is %d ms .... ", u32DelayTime);
    if(u32DelayTime == 300)
        printf("PASS.\n");
    else
        printf("FAIL.\n");

    TIMER_SET_CMP_VALUE(TIMER1, 0xFFFFFF);
    TIMER_Delay(TIMER0, 400000);
    u32DelayTime = TIMER_GetCounter(TIMER1) / 1000;
    printf("    Check DelayTime-4 is %d ms .... ", u32DelayTime);
    if(u32DelayTime == 400)
        printf("PASS.\n");
    else
        printf("FAIL.\n");

    TIMER_SET_CMP_VALUE(TIMER1, 0xFFFFFF);
    TIMER_Delay(TIMER0, 500000);
    u32DelayTime = TIMER_GetCounter(TIMER1) / 1000;
    printf("    Check DelayTime-5 is %d ms .... ", u32DelayTime);
    if(u32DelayTime == 500)
        printf("PASS.\n");
    else
        printf("FAIL.\n");

    printf("\n*** Check TIMER_Delay API delay time done ***\n");

    while(1);
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
