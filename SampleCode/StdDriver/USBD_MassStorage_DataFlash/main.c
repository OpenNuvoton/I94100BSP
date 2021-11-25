/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 2 $
 * $Date: 18/03/05 4:00p $
 * @brief    Please refer readme.txt
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "Platform.h"
#include "massstorage.h"
#include "ConfigSysClk.h"

// Pre-declare function.
void FMC_Initiate(void);
void MSC_Initiate(void);
void HIRC_AutoTrim_Init(void);
void HIRC_AutoTrim_RefSof(void);
extern volatile uint8_t g_usbd_Configured;

// main ========================================================================================================= 
int main(void)
{
	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	// Initiate FMC hardware IP
	FMC_Initiate();
	// Initiate MSC(include USBD hardware IP)
	MSC_Initiate();
	
	while(1)
	{
		if( MSC_IsSuspended() )
		{
			SYS_Unlock();
			PA->MODE = 0xFFFFFFFF;   
			PB->MODE = 0x03FFFFFF;   
			PC->MODE = 0xFFFFFFFF;  
			PD->MODE = 0xFFFFFFFF;  

			// Enable BOD and Low Voltage Reset
			SYS->BODCTL |= (BIT0 | BIT7);		 
			// Keep standard voltage operating
			CLK->LDOCTL &= ~CLK_LDOCTL_OVEN_Msk;//LDO set normal 1.2V
			// Processor is selected to Sleep mode
			SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
			SYS->BODCTL = 0x00000000; 

			// Chip will not enter power-down when sleep after WFI command
			CLK->PWRCTL &= ~CLK_PWRCTL_PDEN_Msk;
			// Disable clock to reduce current consumption
			CLK->AHBCLK = 0;	
			CLK->APBCLK1 = 0;						

			// Force Power down here 
			NVIC_EnableIRQ(PWRWU_IRQn);
			// Enable PD wake-up interrupt
			CLK->PWRCTL |= CLK_PWRCTL_PDWKIEN_Msk;

			// Chip enters Power-down mode when the both PDWTCPU and PDEN bits are set to 1 and CPU runs WFI instruction.
			// This bit is wrute protect
			CLK->PWRCTL |= CLK_PWRCTL_PDWTCPU_Msk;	
			CLK->PWRCTL |= CLK_PWRCTL_PDEN_Msk;
			// Processor is selected to Deep Sleep mode 
			SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;

			// LLPD mode 	
			CLK->PMUCTL &= ~CLK_PMUCTL_PDMSEL_Msk;		
			CLK->PMUCTL |= (0x1 << 0); 
			
			__WFI();				
		}
	}
}
void PWRWU_IRQHandler(void)
{
	/* Check system power down mode wake-up interrupt status flag */
	if(CLK->PWRCTL & CLK_PWRCTL_PDWKIF_Msk) 
	{
		/* Clear system power down wake-up interrupt flag */	
		CLK->PWRCTL |= CLK_PWRCTL_PDWKIF_Msk;
	}		
}
// FMC ========================================================================================================== 
void FMC_Initiate(void)
{
    // Unlock protected registers
    SYS_UnlockReg();
    // Enable FMC ISP function.
    FMC_ENABLE_ISP();
    // Enable APROM update function
    FMC_ENABLE_AP_UPDATE();
}
// USBD and MSC ================================================================================================= 
#define USBD_MSC_PIN_MASK (SYS_GPB_MFPH_PB13MFP_Msk|SYS_GPB_MFPH_PB14MFP_Msk|SYS_GPB_MFPH_PB15MFP_Msk)
#define USBD_MSC_PIN      (SYS_GPB_MFPH_PB13MFP_USBD_DN|SYS_GPB_MFPH_PB14MFP_USBD_DP|SYS_GPB_MFPH_PB15MFP_USBD_VBUS)

void MSC_Initiate(void)
{
	// gpio multi-function configuration.
	SYS->GPB_MFPH = (SYS->GPB_MFPH&(~USBD_MSC_PIN_MASK))|USBD_MSC_PIN;
	// Enable USBD module clock.
	CLK_EnableModuleClock(USBD_MODULE);	
	// Set USBD clock divid
	CLK_SetModuleClock(USBD_MODULE,CLK_CLKSEL4_USBSEL_PLL,CLK_CLKDIV0_USBD(4));	
	// Initiate USBD hardware IP and input MSC request for hand-shake.
	USBD_Open(&gsInfo, MSC_ClassRequest, NULL);
	
	//If using HIRC as PLL clock source.
	#if (SYSCLK_HCLK_CLK == SYSCLK_HCLK_CLK_PLL_HIRC)
	// HIRC Aut Trim initiate and set Config Callback function to trim HIRC.
	HIRC_AutoTrim_Init();
	USBD_SetConfigCallback((SET_CONFIG_CB) HIRC_AutoTrim_RefSof);
	#endif
	
	// Set mass storage class configuration for callback.
	USBD_SetConfigCallback(MSC_SetConfig);	
	// Initiate mass storage for endpoint configuration 
	MSC_Init();
	// Enable USB IRQ
	NVIC_EnableIRQ(USBD_IRQn);
	// Start USBD for processing.
	USBD_Start();
    while(g_usbd_Configured==0);
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
/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
