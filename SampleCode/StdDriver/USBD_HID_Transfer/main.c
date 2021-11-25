/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 18/01/30 4:43p $
 * @brief    Please refer readme.txt
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h> 
#include "Platform.h"
#include "hid_trans.h"
#include "ConfigSysClk.h"

#define DEMO_BUFSIZE     (HIDTRANS_SECTORSIZE*8)

// Test demo buffer to upload/download through HID report
uint8_t g_u8DemoBuffer[DEMO_BUFSIZE] = {0};    

// Pre-declare function.
void UART0_Initiate(void);
void HIRC_AutoTrim_Init(void);
void HIRC_AutoTrim_RefSof(void);

// HID transfer command function ================================================================================ 
void HIDTrans_EraseSector(uint32_t u32StartSector,uint32_t u32Sectors)
{
	printf("  >> Get erase secore request -\r\n");
	printf("     - Start sector number : %d\r\n", u32StartSector);
	printf("     - Erase sector counts : %d\r\n", u32Sectors);
	
	memset(g_u8DemoBuffer+u32StartSector*HIDTRANS_SECTORSIZE, 0xFF, u32Sectors*HIDTRANS_SECTORSIZE);
}
// Provide user prepare read buffer for USB request.
void HIDTrans_PrepareReadPage(uint32_t* pu32Address,uint32_t u32StartPage,uint32_t u32Pages)
{	
	printf("  >> Get read page request -\r\n");
	printf("     - Start page number : %d\r\n", u32StartPage);
	printf("     - Read page counts  : %d\r\n", u32Pages);
	
	if( (u32Pages>0) && ((u32StartPage+u32Pages)<DEMO_BUFSIZE/HIDTRANS_PAGESIZE) )
		*pu32Address = (uint32_t)g_u8DemoBuffer + u32StartPage*HIDTRANS_PAGESIZE;
	else
		*pu32Address = NULL;
}
// Provide user prepare write buffer for USB request.
void HIDTrans_PrepareWritePage(uint32_t* pu32Address,uint32_t u32StartPage,uint32_t u32Pages)
{
	printf("  >> Get write page request -\r\n");
	printf("     - Start page number : %d\r\n", u32StartPage);
	printf("     - Write page counts : %d\r\n", u32Pages);
	
	if( (u32Pages>0) && ((u32StartPage+u32Pages)<DEMO_BUFSIZE/HIDTRANS_PAGESIZE) )
		*pu32Address = (uint32_t)g_u8DemoBuffer + u32StartPage*HIDTRANS_PAGESIZE;
	else
		*pu32Address = NULL;
}
// Provide user get write data.
void HIDTrans_GetWriteData(uint32_t u32Address,uint32_t u32Pages)
{
	printf("  >> Get write data finish message.\r\n");
	printf("     - Write page counts : %d\r\n", u32Pages);	
}

// main ========================================================================================================= 
#define USBD_HID_PIN_MASK (SYS_GPB_MFPH_PB13MFP_Msk|SYS_GPB_MFPH_PB14MFP_Msk|SYS_GPB_MFPH_PB15MFP_Msk)
#define USBD_HID_PIN      (SYS_GPB_MFPH_PB13MFP_USBD_DN|SYS_GPB_MFPH_PB14MFP_USBD_DP|SYS_GPB_MFPH_PB15MFP_USBD_VBUS)

int main(void)
{	
	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();
	// Initiate UART0 for printf
	UART0_Initiate();

	printf("\r\n+-------------------------------------------------+\r\n");
	printf("|          HID Transfer Demo Sample Code          |\r\n");
	printf("+-------------------------------------------------+\r\n");
	
	// Initiate HID Transfer(include USBD hardware IP)
	{
		// gpio multi-function configuration.
		SYS->GPB_MFPH = (SYS->GPB_MFPH&(~USBD_HID_PIN_MASK))|USBD_HID_PIN;
		// Enable USBD module clock.
		CLK_EnableModuleClock(USBD_MODULE);
		// Set USBD clock divid
		CLK_SetModuleClock(USBD_MODULE,CLK_CLKSEL4_USBSEL_PLL,CLK_CLKDIV0_USBD(4));	
		// Initiate USBD hardware IP and input HID request for hand-shake.
		USBD_Open(&gsInfo, HIDTrans_ClassRequest, NULL);
		
		//If using HIRC as PLL clock source.
		#if (SYSCLK_HCLK_CLK == SYSCLK_HCLK_CLK_PLL_HIRC)
		// HIRC Aut Trim initiate and set Config Callback function to trim HIRC.
		HIRC_AutoTrim_Init();
		USBD_SetConfigCallback((SET_CONFIG_CB) HIRC_AutoTrim_RefSof);
		#endif
		
		// Initiate HID for endpoint configuration 
		HIDTrans_Initiate();
		// Enable USB IRQ
		NVIC_EnableIRQ(USBD_IRQn);
		// Start USBD for processing.
		USBD_Start();
	}
	// Process in iterrupt.
	while(1);
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

// UART0 ======================================================================================================== 
#define UART0_BAUDRATE     (115200)
#define UART0_PIN_MASK (SYS_GPB_MFPH_PB8MFP_Msk|SYS_GPB_MFPH_PB9MFP_Msk)
#define UART0_PIN      (SYS_GPB_MFPH_PB8MFP_UART0_TXD|SYS_GPB_MFPH_PB9MFP_UART0_RXD)

void UART0_Initiate(void)
{
    // Enable UART module clock 
    CLK_EnableModuleClock(UART0_MODULE);
    // Select UART module clock source as HXT and UART module clock divider as 1
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));
	// Configure UART0 and set UART0 Baud rate
    UART_Open(UART0, UART0_BAUDRATE);
    // Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8)
    SYS->GPB_MFPH = (SYS->GPB_MFPH&~UART0_PIN_MASK)|UART0_PIN;
}

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
