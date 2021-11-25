/******************************************************************************
* @file     main.c
* @version  V1.00
* $Revision: 1 $
* $Date: 17/12/21 11:31p $
* @brief    Demonstrate how to use LXT or USBSOF(USB Start Of Frame)to trim HIRC
*
* @note
* Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include "stdio.h"
#include "Platform.h"
#include "hid_request.h"
#include "ConfigSysClk.h"

#define TRIMMODE_48M_LXT       (0)
#define TRIMMODE_49M_LXT       (1)
#define TRIMMODE_48M_USBSOF    (2)
#define TRIMMODE_49M_USBSOF    (3) 
// SYS and CLK ==================================================================================================
#define CLK_X32_PIN_MSK   (SYS_GPC_MFPL_PC0MFP_Msk | SYS_GPC_MFPL_PC1MFP_Msk)  
#define CLK_X32_PIN       (SYS_GPC_MFPL_PC0MFP_X32_OUT | SYS_GPC_MFPL_PC1MFP_X32_IN)

// Pre-declare function.
void UART0_Initiate(void);
void TrimHIRC(UINT8 u8Mode);
void CalTrimFreqViaTimer0(void);

// main ========================================================================================================= 
int32_t main (void)
{
	UINT8 u8Lock, u8Item = 0;

	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();
	
	// Set PC multi-function pins for X32_OUT(PC.0) and X32_IN(PC.1)
	SYS->GPC_MFPL = (SYS->GPC_MFPL&~(CLK_X32_PIN_MSK))|CLK_X32_PIN;
	
	u8Lock = SYS_Unlock();
	// Enable HXT and LXT clock. LXT: For trim HIRC. HXT: For Systick to measure result.
	CLK_EnableXtalRC(CLK_PWRCTL_LXTEN_Msk | CLK_PWRCTL_HXTEN_Msk);
	// Wait for HIRC, HXT and LXT clock ready
	CLK_WaitClockReady(CLK_STATUS_LXTSTB_Msk | CLK_STATUS_HXTSTB_Msk);
	SYS_Lock(u8Lock);

	// Initiate UART0 for printf
	UART0_Initiate();
	
	do
	{
		printf("\r\n+-------------------------------------------------+\r\n");
		printf("|   HIRC Auto trim 32k X'Tal or USB Sample Code   |\r\n");
		printf("+-------------------------------------------------+\r\n");
		printf("  >> Please plug 32k X'Tal first << \r\n");
		printf("  [1] HIRC =48M hz , Clock source 32k X'Tal \r\n");
		printf("  [2] HIRC =49.152M hz , Clock source 32k X'Tal \r\n");
		printf("  >> Please connect USB  first << \r\n");
		printf("  [3] HIRC =48M hz , Clock source USB \r\n");
		printf("  [4] HIRC =49.152M hz , Clock source USB \r\n");
		
		u8Item = getchar();
		if( (u8Item>='1') && (u8Item<='4') )
		{
			TrimHIRC((u8Item-'1'));
			CalTrimFreqViaTimer0();		
			printf("  >> End Trim HIRC demo \r\n");
			while(1);
		}
		printf("  >> Unknow selection.. \r\n\r\n");
	}while(1);
}

void IRC_IRQHandler()
{
	// Get Trim Failure Interrupt
    if(SYS_GET_TRIMHIRC_INT_FLAG(SYS_IRCTISTS_TRIMFAIL_INT_FLAG))
	{ 
        // Display HIRC trim status(trim fail) 
        printf("HIRC Trim Failure Interrupt\r\n");
        // Clear Trim Failure Interrupt
		SYS_CLEAR_TRIMHIRC_INT_FLAG(SYS_IRCTISTS_TRIMFAIL_INT_FLAG);
    }
	// Get LXT Clock Error Interrupt
    if(SYS_GET_TRIMHIRC_INT_FLAG(SYS_IRCTISTS_CLKERROR_INT_FLAG)) 
	{ 
        // Display HIRC trim status(LXT clock error) 
        printf("LXT Clock Error Interrupt\r\n");
        // Clear LXT Clock Error Interrupt 
        SYS_CLEAR_TRIMHIRC_INT_FLAG(SYS_IRCTISTS_CLKERROR_INT_FLAG);
    }

}
// HIRCTrim Process =============================================================================================
#define USBD_HID_PIN_MASK (SYS_GPB_MFPH_PB13MFP_Msk|SYS_GPB_MFPH_PB14MFP_Msk|SYS_GPB_MFPH_PB15MFP_Msk)
#define USBD_HID_PIN      (SYS_GPB_MFPH_PB13MFP_USBD_DN|SYS_GPB_MFPH_PB14MFP_USBD_DP|SYS_GPB_MFPH_PB15MFP_USBD_VBUS)

void TrimHIRC(UINT8 u8Mode)
{
	if( (u8Mode==TRIMMODE_48M_USBSOF) || (u8Mode==TRIMMODE_49M_USBSOF) )
	{
		// gpio multi-function configuration.
		SYS->GPB_MFPH = (SYS->GPB_MFPH&(~USBD_HID_PIN_MASK))|USBD_HID_PIN;
		// Enable USBD module clock.
		CLK_EnableModuleClock(USBD_MODULE);
		// Set USBD clock divide
		CLK_SetModuleClock(USBD_MODULE, CLK_CLKSEL4_USBSEL_PLL, CLK_CLKDIV0_USBD(4));
		// Initiate USBD hardware IP and input HID request for hand-shake.
		USBD_Open(&gsInfo, HID_ClassRequest, NULL);
		// Initiate mass storage for endpoint configuration 
		HID_Init();
		// Enable USB IRQ
		NVIC_EnableIRQ(USBD_IRQn);
		// Start USBD for processing.
		USBD_Start();
		// Wait USB is attached
		while(USBD_IS_ATTACHED()!=TRUE);
	}
	
	// Enable NVIC Interrupt
	NVIC_EnableIRQ(IRC_IRQn);
	// Unlock protected registers
	SYS_UnlockReg();
	// Enable clock error & trim fail interrupt.
	SYS_ENABLE_TRIMHIRC_INT(SYS_IRCTIEN_TRIMFAIL_INT_MASK|SYS_IRCTIEN_CLKERROR_INT_MASK);
	// Set trim calculation loop selection 
	SYS_SET_TRIMHIRC_LOOPSEL(SYS_IRCTCTL_LOOPSEL_4);
	// Set trim value update limitation count
	SYS_SET_TRIMHIRC_RETRYCNT(SYS_IRCTCTL_RETRYCNT_64);
	// Enable clock error stop.
	SYS_ENABLE_TRIMHIRC_CLKERRSTOP();
	// Set reference clock selection.
	SYS_SET_TRIMHIRC_REFCLK(((u8Mode==TRIMMODE_48M_LXT)||(u8Mode==TRIMMODE_49M_LXT))?SYS_IRCTCTL_REFCLK_LXT:SYS_IRCTCTL_REFCLK_USBSOF);
	// Enable IRC trim and set reference clock selection.
	SYS_EnableTrimHIRC(((u8Mode==TRIMMODE_48M_LXT)||(u8Mode==TRIMMODE_48M_USBSOF))?SYS_IRCTCTL_FREQSEL_48M:SYS_IRCTCTL_FREQSEL_49M);
	// Waiting for HIRC Frequency Lock 
	CLK_SysTickDelay(4000); 
	
	// Get HIRC Frequency Lock
	while(1) 
	{
		if(SYS_IS_TRIMHIRC_DONE())
		{
			printf("\r\n  >> HIRC Frequency Lock. \r\n");
			break;
		}
	}
	// Disable IRC Trim
	SYS_DisableTrimHIRC();
	// Disable NVIC Interrupt
	NVIC_DisableIRQ(IRC_IRQn);
	// Lock protected registers. 
	SYS_LockReg();		
}
// Get Trimed Frequency =========================================================================================
#define TIMR0_PRESCALE    (5)

volatile uint32_t u32TrimedFreq = 0;

void CalTrimFreqViaTimer0(void)
{
    // Unlock protected registers
	SYS_UnlockReg();
	// Enable Timer0 module clock.
	CLK_EnableModuleClock(TMR0_MODULE);
	// Set Timer0 module clock source.
	CLK_SetModuleClock(TMR0_MODULE, CLK_CLKSEL1_TMR0SEL_HIRC, 0);
	// Open Timer0 hardware ip.
	TIMER_Open(TIMER0,TIMER_ONESHOT_MODE,__HIRC);
	// Set compare value directly.
	TIMER_SET_CMP_VALUE(TIMER0,0xffffff);
	// Set prescale value for divide.
	TIMER_SET_PRESCALE_VALUE(TIMER0,(TIMR0_PRESCALE-1));
	// Start counte
	TIMER_Start(TIMER0);
	// Enable sys tick for trigger time.
	CLK_EnableSysTick(CLK_CLKSEL0_STCLKSEL_HXT,__HXT);	
	// Check already trigger irq.
	while(u32TrimedFreq==0);
	// Display current trimed frequency.
	printf("  >> After Trim HIRC, Current CPU @ %dHz \r\n",u32TrimedFreq);
	// Protected registers
	SYS_LockReg();
}

void SysTick_Handler()
{
	// Get Timer0 counter.
	u32TrimedFreq=(TIMER_GetCounter(TIMER0))*TIMR0_PRESCALE;
	// Disable systick interrupt.
	CLK_DisableSysTick();
}
// UART0 ======================================================================================================== 
#define UART0_BAUDRATE     (115200)
#define UART0_MSC_PIN_MASK (SYS_GPB_MFPH_PB8MFP_Msk|SYS_GPB_MFPH_PB9MFP_Msk)
#define UART0_MSC_PIN      (SYS_GPB_MFPH_PB8MFP_UART0_TXD|SYS_GPB_MFPH_PB9MFP_UART0_RXD)

void UART0_Initiate(void)
{
	// Enable UART module clock 
	CLK_EnableModuleClock(UART0_MODULE);
	// Select UART module clock source as HXT and UART module clock divider as 1
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));
	// Configure UART0 and set UART0 Baud rate
	UART_Open(UART0, UART0_BAUDRATE);
	// Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8)
	SYS->GPB_MFPH = (SYS->GPB_MFPH&~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk))
									|(SYS_GPB_MFPH_PB8MFP_UART0_TXD|SYS_GPB_MFPH_PB9MFP_UART0_RXD);
}
