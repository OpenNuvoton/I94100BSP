/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/03/14 10:04a $
 * @brief    Get the current RTC data/time per tick.
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "Platform.h"
#include "ConfigSysClk.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Global Interface Variables Declarations                                                                 */
/*---------------------------------------------------------------------------------------------------------*/
volatile uint32_t g_u32RTCTickINT;


/**
 * @brief       IRQ Handler for RTC Interrupt
 *
 * @param       None
 *
 * @return      None
 *
 * @details     The RTC_IRQHandler is default IRQ of RTC, declared in startup_M451Series.s.
 */
void RTC_IRQHandler(void)
{
    /* To check if RTC tick interrupt occurred */
    if(RTC_GET_TICK_INT_FLAG() == 1)
    {
        /* Clear RTC tick interrupt flag */
        RTC_CLEAR_TICK_INT_FLAG();

        g_u32RTCTickINT++;

        PB8 ^= 1;
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
    S_RTC_TIME_DATA_T sWriteRTC, sReadRTC;
    uint32_t u32Sec;
    uint8_t u8Lock, u8IsNewDateTime = 0;

		// Initiate system clock(Configure in ConfigSysClk.h)
		SYSCLK_INITIATE();

		/* Init UART0 for printf */
		UART0_Init();

		printf("\n\nCPU @ %dHz\n", SystemCoreClock);
		printf("+-----------------------------------------+\n");
		printf("|    RTC Date/Time and Tick Sample Code   |\n");
		printf("+-----------------------------------------+\n\n");

		// Unlock register
		u8Lock = SYS_Unlock();
		/* Enable LXT-32KHz for RTC */
		CLK_EnableXtalRC(CLK_PWRCTL_LXTEN_Msk);
		/* Waiting for clock ready */
		CLK_WaitClockReady(CLK_STATUS_LXTSTB_Msk);
		// Lock register
		SYS_Lock(u8Lock);

		// Set RTC clock source to LXT.
		CLK_SetModuleClock(RTC_MODULE, CLK_CLKSEL3_RTCSEL_LXT, 0);

		/* Enable peripheral clock */
		CLK_EnableModuleClock(RTC_MODULE);

		/* Enable RTC NVIC */
		NVIC_EnableIRQ(RTC_IRQn);

    /* Open RTC and start counting */
    sWriteRTC.u32Year       = 2014;
    sWriteRTC.u32Month      = 5;
    sWriteRTC.u32Day        = 15;
    sWriteRTC.u32DayOfWeek  = RTC_THURSDAY;
    sWriteRTC.u32Hour       = 15;
    sWriteRTC.u32Minute     = 30;
    sWriteRTC.u32Second     = 30;
    sWriteRTC.u32TimeScale  = RTC_CLOCK_24;
    RTC_Open(&sWriteRTC);

    /* Enable RTC tick interrupt, one RTC tick is 1/4 second */
    RTC_EnableInt(RTC_INTEN_TICKIEN_Msk);
    RTC_SetTickPeriod(RTC_TICK_1_4_SEC);

    printf("# Showing RTC date/time on UART0.\n\n");
    printf("1.) Use PB.8 to check tick period time is 1/4 second or not.\n");
    printf("2.) Show RTC date/time and change date/time after 5 seconds:\n");

    /* Use PB.8 to check tick period time */
    PB->MODE = (PB->MODE & ~GPIO_MODE_MODE8_Msk) | (GPIO_MODE_OUTPUT << GPIO_MODE_MODE8_Pos);

    u32Sec = 0;
    g_u32RTCTickINT = 0;
    while(1)
    {
        if(g_u32RTCTickINT == 4)
        {
            g_u32RTCTickINT = 0;

            /* Read current RTC date/time */
            RTC_GetDateAndTime(&sReadRTC);
            if(u8IsNewDateTime == 0)
            {
                printf("    %d/%02d/%02d %02d:%02d:%02d\n",
                       sReadRTC.u32Year, sReadRTC.u32Month, sReadRTC.u32Day, sReadRTC.u32Hour, sReadRTC.u32Minute, sReadRTC.u32Second);
            }
            else
            {
                printf("    %d/%02d/%02d %02d:%02d:%02d\r",
                       sReadRTC.u32Year, sReadRTC.u32Month, sReadRTC.u32Day, sReadRTC.u32Hour, sReadRTC.u32Minute, sReadRTC.u32Second);
            }

            if(u32Sec == sReadRTC.u32Second)
            {
                printf("\nRTC tick period time is incorrect.\n");
                while(1);
            }

            u32Sec = sReadRTC.u32Second;

            if(u8IsNewDateTime == 0)
            {
                if(u32Sec == (sWriteRTC.u32Second + 5))
                {
                    printf("\n");
                    printf("3.) Update new date/time to 2014/05/18 11:12:13.\n");

                    u8IsNewDateTime = 1;
                    RTC_SetDate(2014, 5, 18, RTC_SUNDAY);
                    RTC_SetTime(11, 12, 13, RTC_CLOCK_24, RTC_AM);
                }
            }
        }
    }
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
