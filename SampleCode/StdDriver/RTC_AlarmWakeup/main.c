/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/03/14 10:04a $
 * @brief    Please refer readme.txt
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
volatile uint8_t g_u8IsRTCAlarmINT = 0;

/**
 * @brief       IRQ Handler for RTC Interrupt
 *
 * @param       None
 *
 * @return      None
 *
 * @details     The RTC_IRQHandler is default IRQ of RTC, declared in startup .s.
 */
void RTC_IRQHandler(void)
{
    /* To check if RTC alarm interrupt occurred */
    if((RTC_GET_ALARM_INT_FLAG() == 1) && (RTC_GET_ALARM_INT_EN() == 1))
    {
        /* Clear RTC alarm interrupt flag */
        RTC_CLEAR_ALARM_INT_FLAG();

        g_u8IsRTCAlarmINT++;
    }
}

void UART0_Init(void)
{
	/* Enable peripheral clock */
	CLK_EnableModuleClock(UART0_MODULE);

	/* Select UART module clock source as PLL and UART module clock divider as 1 */
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
	S_RTC_TIME_DATA_T sWriteRTC, sReadRTC;
	uint8_t u8Lock;

	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	/* Init UART0 for printf */
	UART0_Init();

	printf("\n\nCPU @ %dHz\n", SystemCoreClock);
	printf("+-------------------------------------+\n");
	printf("|    RTC Alarm Wake-up Sample Code    |\n");
	printf("+-------------------------------------+\n\n");

	// Unlock register
	u8Lock = SYS_Unlock();
	/* Enable LXT-32KHz for RTC */
	CLK_EnableXtalRC(CLK_PWRCTL_LXTEN_Msk);
	/* Waiting for clock ready */
	CLK_WaitClockReady(CLK_STATUS_LXTSTB_Msk);
	// Lock register
	SYS_Lock(u8Lock);
	
	/* Set SysTick source to HCLK/2*/
	CLK_SetSysTickClockSrc(CLK_CLKSEL0_STCLKSEL_HCLK_DIV2);
	CLK_SetModuleClock(RTC_MODULE, CLK_CLKSEL3_RTCSEL_LXT, 0);
	/* Enable peripheral clock */
	CLK_EnableModuleClock(RTC_MODULE);


	/* Open RTC */
	sWriteRTC.u32Year       = 2014;
	sWriteRTC.u32Month      = 5;
	sWriteRTC.u32Day        = 15;
	sWriteRTC.u32DayOfWeek  = RTC_THURSDAY;
	sWriteRTC.u32Hour       = 23;
	sWriteRTC.u32Minute     = 59;
	sWriteRTC.u32Second     = 50;
	sWriteRTC.u32TimeScale  = RTC_CLOCK_24;
	RTC_Open(&sWriteRTC);

	/* Set RTC alarm date/time */
	sWriteRTC.u32Year       = 2014;
	sWriteRTC.u32Month      = 5;
	sWriteRTC.u32Day        = 15;
	sWriteRTC.u32DayOfWeek  = RTC_THURSDAY;
	sWriteRTC.u32Hour       = 23;
	sWriteRTC.u32Minute     = 59;
	sWriteRTC.u32Second     = 55;
	RTC_SetAlarmDateAndTime(&sWriteRTC);

	/* Enable RTC alarm interrupt and wake-up function will be enabled also */
	RTC_EnableInt(RTC_INTEN_ALMIEN_Msk);

	/* Enable RTC NVIC */
	NVIC_EnableIRQ(RTC_IRQn);

	printf("# Set RTC current date/time: 2014/05/15 23:59:50.\n");
	printf("# Set RTC alarm date/time:   2014/05/15 23:59:55.\n");
	printf("# Wait system waken-up by RTC alarm interrupt event.\n");

	g_u8IsRTCAlarmINT = 0;

	/* System enter to Power-down */
	/* To program PWRCTL register, it needs to disable register protection first. */
	SYS_UnlockReg();
	printf("\nSystem enter to power-down mode ...\n");
	/* To check if all the debug messages are finished */
	while(IsDebugFifoEmpty() == 0);
	CLK_PowerDown();

	while(g_u8IsRTCAlarmINT == 0);

	/* Read current RTC date/time */
	RTC_GetDateAndTime(&sReadRTC);
	printf("System has been waken-up and current date/time is:\n");
	printf("    %d/%02d/%02d %02d:%02d:%02d\n",
				 sReadRTC.u32Year, sReadRTC.u32Month, sReadRTC.u32Day, sReadRTC.u32Hour, sReadRTC.u32Minute, sReadRTC.u32Second);
				 
	printf("\n\n");
	printf("# Set next RTC alarm date/time: 2014/05/16 00:00:05.\n");
	printf("# Wait system waken-up by RTC alarm interrupt event.\n");
	RTC_SetAlarmDate(2014, 05, 16);
	RTC_SetAlarmTime(0, 0, 5, RTC_CLOCK_24, 0);

	g_u8IsRTCAlarmINT = 0;

	/* System enter to Power-down */
	/* To program PWRCTL register, it needs to disable register protection first. */
	SYS_UnlockReg();
	printf("\nSystem enter to power-down mode ...\n");
	/* To check if all the debug messages are finished */
	while(IsDebugFifoEmpty() == 0);
	CLK_PowerDown();

	while(g_u8IsRTCAlarmINT == 0);

	/* Read current RTC date/time */
	RTC_GetDateAndTime(&sReadRTC);
	printf("System has been waken-up and current date/time is:\n");
	printf("    %d/%02d/%02d %02d:%02d:%02d\n",
				 sReadRTC.u32Year, sReadRTC.u32Month, sReadRTC.u32Day, sReadRTC.u32Hour, sReadRTC.u32Minute, sReadRTC.u32Second);

	while(1);
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
