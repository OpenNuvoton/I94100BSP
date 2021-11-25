/******************************************************************************
 * @file     main.c
 * @brief    Please refer readme.txt
 * @version  1.0.0
 * @date     2018/04/03
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "printer_hidtrans.h"
#include "ConfigSysClk.h"

// Define VCOM & HIDTrans variable.
#define HIDTRANS_BUFSIZE          (HIDTRANS_SECTORSIZE*8)

// Test demo buffer to upload/download through HID report
uint8_t   g_u8DemoBuffer[HIDTRANS_BUFSIZE] = {0};    

// Pre-declare function.
void Printer_And_HIDTrans_Initiate(void);
void HIRC_AutoTrim_Init(void);
void HIRC_AutoTrim_RefSof(void);

// main ========================================================================================================= 
int32_t main (void)
{
	// Initiate system clock(Configuration in ConfigSysClk.h)
	SYSCLK_INITIATE();

	// Initiate Printer, HID transfer and USBD hardware IP.
    Printer_And_HIDTrans_Initiate();
	
	// Process in interrupt
    while(1) 
	{
    };
}

// HID transfer command callback function ======================================================================= 
// Provide user to erase sector for processing.
void HIDTrans_EraseSector(uint32_t u32StartSector,uint32_t u32Sectors)
{
	memset(g_u8DemoBuffer+u32StartSector*HIDTRANS_SECTORSIZE, 0xFF, u32Sectors*HIDTRANS_SECTORSIZE);
}
// Provide user prepare read buffer for USB request.
void HIDTrans_PrepareReadPage(uint32_t* pu32Address,uint32_t u32StartPage,uint32_t u32Pages)
{	
	if( (u32Pages>0) && ((u32StartPage+u32Pages)<HIDTRANS_BUFSIZE/HIDTRANS_PAGESIZE) )
		*pu32Address = (uint32_t)g_u8DemoBuffer + u32StartPage*HIDTRANS_PAGESIZE;
	else
		*pu32Address = NULL;
}
// Provide user prepare write buffer for USB request.
void HIDTrans_PrepareWritePage(uint32_t* pu32Address,uint32_t u32StartPage,uint32_t u32Pages)
{
	if( (u32Pages>0) && ((u32StartPage+u32Pages)<HIDTRANS_BUFSIZE/HIDTRANS_PAGESIZE) )
		*pu32Address = (uint32_t)g_u8DemoBuffer + u32StartPage*HIDTRANS_PAGESIZE;
	else
		*pu32Address = NULL;
}
// Provide user get write data.
void HIDTrans_GetWriteData(uint32_t u32Address,uint32_t u32Pages)
{
}

// Micro printer callback function ============================================================================== 
// Provide user receive data from PC.
void Printer_ReceiveData(uint8_t* pu8DataBuf, uint32_t u32DataCount)
{
	// Different printers will receive different structures.
}

// Printer and HID transfer ===================================================================================== 
#define PRINTER_AND_HIDTRANS_USBD_PIN_MASK     (SYS_GPB_MFPH_PB13MFP_Msk|SYS_GPB_MFPH_PB14MFP_Msk|SYS_GPB_MFPH_PB15MFP_Msk)
#define PRINTER_AND_HIDTRANS_USBD_PIN          (SYS_GPB_MFPH_PB13MFP_USBD_DN|SYS_GPB_MFPH_PB14MFP_USBD_DP|SYS_GPB_MFPH_PB15MFP_USBD_VBUS)

void Printer_And_HIDTrans_Initiate(void)
{
	// Enable USBD module clock.
	CLK_EnableModuleClock(USBD_MODULE);
	// Set USBD clock divid
	CLK_SetModuleClock(USBD_MODULE, CLK_CLKSEL4_USBSEL_PLL, CLK_CLKDIV0_USBD(4));	
	// Initiate USBD hardware IP and input HID request for hand-shake.
	USBD_Open(&gsInfo, Printer_HIDTrans_ClassRequest, NULL);
	
	//If using HIRC as PLL clock source.
	#if (SYSCLK_HCLK_CLK == SYSCLK_HCLK_CLK_PLL_HIRC)
	// HIRC Aut Trim initiate and set Config Callback function to trim HIRC.
	HIRC_AutoTrim_Init();
	USBD_SetConfigCallback((SET_CONFIG_CB) HIRC_AutoTrim_RefSof);
	#endif
	
	// Initiate endpoint configuration of VCOM & HID 
	Printer_HIDTrans_Init();
	// Enable USB IRQ
	NVIC_EnableIRQ(USBD_IRQn);
	// Start USBD for processing.
	USBD_Start();	
	// gpio multi-function configuration.
	SYS->GPB_MFPH = (SYS->GPB_MFPH&(~PRINTER_AND_HIDTRANS_USBD_PIN_MASK))|PRINTER_AND_HIDTRANS_USBD_PIN;	
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
