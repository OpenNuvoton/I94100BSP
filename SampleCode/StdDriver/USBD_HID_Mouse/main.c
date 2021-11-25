/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 2 $
 * $Date:    18/04/10 9:36a $
 * @brief    Please refer readme.txt
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h> 
#include "Platform.h"
#include "hid_mouse.h"
#include "ConfigSysClk.h"

//----------------------------------------------------------------------------
//  Functions Definition
//----------------------------------------------------------------------------
void HIRC_AutoTrim_Init(void);
void HIRC_AutoTrim_RefSof(void);

// main ========================================================================================================= 
#define USBD_HID_PIN_MASK (SYS_GPB_MFPH_PB13MFP_Msk|SYS_GPB_MFPH_PB14MFP_Msk|SYS_GPB_MFPH_PB15MFP_Msk)
#define USBD_HID_PIN      (SYS_GPB_MFPH_PB13MFP_USBD_DN|SYS_GPB_MFPH_PB14MFP_USBD_DP|SYS_GPB_MFPH_PB15MFP_USBD_VBUS)

int main(void)
{	
	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	// Initiate HID mouse(include USBD hardware IP)
	{
		// gpio multi-function configuration.
		SYS->GPB_MFPH = (SYS->GPB_MFPH&(~USBD_HID_PIN_MASK))|USBD_HID_PIN;
		// Enable USBD module clock.
		CLK_EnableModuleClock(USBD_MODULE);
		// Set USBD clock divid
		CLK_SetModuleClock(USBD_MODULE, CLK_CLKSEL4_USBSEL_PLL, CLK_CLKDIV0_USBD(4));
		// Reset module.
		SYS_ResetModule(USBD_RST);
		// Initiate USBD hardware IP and input HID request for hand-shake.
		USBD_Open(&gsInfo, HID_ClassRequest, NULL);

		//If using HIRC as PLL clock source.
		#if (SYSCLK_HCLK_CLK == SYSCLK_HCLK_CLK_PLL_HIRC)
		// HIRC Aut Trim initiate and set Config Callback function to trim HIRC.
		HIRC_AutoTrim_Init();
		USBD_SetConfigCallback((SET_CONFIG_CB) HIRC_AutoTrim_RefSof);
		#endif
		
		// Initiate HID for endpoint configuration 
		HID_Init();
		// Enable USB IRQ
		NVIC_EnableIRQ(USBD_IRQn);
		// Start USBD for processing.
		USBD_Start();
	}
	// Process in main loop.
	while(1)
	{
		HID_UpdateMouseData();
	}
}

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
	// HIRC auto trim enable to trim 48MHz.
	SYS_EnableTrimHIRC(SYS_IRCTCTL_FREQSEL_48M);

	while(!SYS_IS_TRIMHIRC_DONE())
	{
	}
	
	SYS_CLEAR_TRIMHIRC_INT_FLAG(SYS_IRCTISTS_TRIMFAIL_INT_FLAG|SYS_IRCTISTS_CLKERROR_INT_FLAG);
}

/*** (C) COPYRIGHT 2018 Nuvoton Technology Corp. ***/
