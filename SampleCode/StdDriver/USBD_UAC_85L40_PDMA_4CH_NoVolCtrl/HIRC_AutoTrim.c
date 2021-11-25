/******************************************************************************
 * @file     HIRC_AutoTrim.c
 * @brief    USBD driver Sample file
 *
 * @note
 * Copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

/*!<Includes */
#include <stdio.h>
#include <string.h>
#include "Platform.h"

extern volatile uint8_t g_usbd_Configured;

#define IRCTCTL_AUTOTRIM48M     (0x1<<0)
#define IRCTCTL_AUTOTRIM49M     (0x3<<0)
#define IRCTCTL_LOOPSEL_4       (0x0<<4)
#define IRCTCTL_LOOPSEL_8       (0x1<<4)
#define IRCTCTL_LOOPSEL_16      (0x2<<4)
#define IRCTCTL_LOOPSEL_32      (0x3<<4)
#define IRCTCTL_RETRYCNT_64		(0x0<<6)
#define IRCTCTL_RETRYCNT_128    (0x1<<6) 
#define IRCTCTL_RETRYCNT_256    (0x2<<6)
#define IRCTCTL_RETRYCNT_512    (0x3<<6)
#define IRCTCTL_CESTOPEN        (0x1<<8)    
#define IRCTCTL_REFCLK_SOF      (0x1<<10)   
    
//----------------------------------------------------------------------------
//  HIRC Trim function
//----------------------------------------------------------------------------
void IRC_IRQHandler(void)
{
	// Get Trim Failure Interrupt
	if(SYS_GET_TRIMHIRC_INT_FLAG(SYS_IRCTISTS_TRIMFAIL_INT_FLAG))
	{ 
		// Clear Trim Failure Interrupt
		SYS_CLEAR_TRIMHIRC_INT_FLAG(SYS_IRCTISTS_TRIMFAIL_INT_FLAG);
	}
	
	// Get LXT Clock Error Interrupt
	if(SYS_GET_TRIMHIRC_INT_FLAG(SYS_IRCTISTS_CLKERROR_INT_FLAG)) 
	{ 
		// Clear LXT Clock Error Interrupt 
		SYS_CLEAR_TRIMHIRC_INT_FLAG(SYS_IRCTISTS_CLKERROR_INT_FLAG);
	}
}

void TMR0_IRQHandler(void)
{
	static uint8_t u8Count = 0;
	
	if(TIMER_GetIntFlag(TIMER0) == 1)
	{
		/* Clear Timer0 time-out interrupt flag */
		TIMER_ClearIntFlag(TIMER0);
	
		if(++u8Count >= 10)
		{
			SYS_EnableTrimHIRC(SYS_IRCTCTL_FREQSEL_48M);
			u8Count = 0;
		}
	}
}

void HIRC_AutoTrim_Init(void)
{
	SYS_SET_TRIMHIRC_LOOPSEL(SYS_IRCTCTL_LOOPSEL_4);
	SYS_SET_TRIMHIRC_RETRYCNT(SYS_IRCTCTL_RETRYCNT_64);
	SYS_ENABLE_TRIMHIRC_CLKERRSTOP();
	SYS_SET_TRIMHIRC_REFCLK(SYS_IRCTCTL_REFCLK_USBSOF);
	
	// Enable clock error / trim fail interrupt 		
	SYS_ENABLE_TRIMHIRC_INT(SYS_IRCTIEN_TRIMFAIL_INT_MASK|SYS_IRCTIEN_CLKERROR_INT_MASK);
	
	NVIC_EnableIRQ(IRC_IRQn);
	
	// Timer Initiate for periodic HIRC Auto Trim.
	/* Enable peripheral clock */
	CLK_EnableModuleClock(TMR0_MODULE);
	/* Peripheral clock source */
	CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_PCLK0, 0);
	
	/* Open Timer0 in periodic mode, enable interrupt and 1 interrupt tick per second */
	TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 1);
	TIMER_EnableInt(TIMER0);
	
	/* Enable Timer0 ~ Timer3 NVIC */
	NVIC_EnableIRQ(TMR0_IRQn);
	
	/* Start Timer0 ~ Timer3 counting */
	TIMER_Start(TIMER0);
}

void HIRC_AutoTrim_RefSof(void)
{																													
	// HIRC auto trim enable/disable 
	SYS_EnableTrimHIRC(SYS_IRCTCTL_FREQSEL_49M);

	while(!SYS_IS_TRIMHIRC_DONE())
	{
	}
	
	SYS_CLEAR_TRIMHIRC_INT_FLAG(SYS_IRCTISTS_TRIMFAIL_INT_FLAG|SYS_IRCTISTS_CLKERROR_INT_FLAG);
}
