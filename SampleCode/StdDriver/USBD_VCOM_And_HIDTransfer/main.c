/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 18/03/22 10:52a $
 * @brief    Please refer readme.txt
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h> 
#include "Platform.h"
#include "vcom_hidtrans.h"
#include "ConfigSysClk.h"

//----------------------------------------------------------------------------
//	Define VCOM & HIDTrans
//----------------------------------------------------------------------------
#define HIDTRANS_BUFSIZE          (HIDTRANS_SECTORSIZE*8)
#define VCOM_UART2USB_BUFSIZE     (128)
#define VCOM_USB2UART_BUFSIZE     (128)

//----------------------------------------------------------------------------
// Define Variable
//----------------------------------------------------------------------------
// Test demo buffer to upload/download through HID report
uint8_t   g_u8DemoBuffer[HIDTRANS_BUFSIZE] = {0};    
volatile S_BUFCTRL g_sUsbToUart,g_sUartToUsb;
int32_t g_i32UartToUsbBuf[VCOM_UART2USB_BUFSIZE];
int32_t g_i32UsbToUartBuf[VCOM_USB2UART_BUFSIZE];
// Extern variable
extern volatile uint8_t g_u8CDC_EP2Empty;
extern S_BUFCTRL *g_psVCOM_InBufCtrl,*g_psVCOM_OutBufCtrl; 

//----------------------------------------------------------------------------
//  Functions Definition
//----------------------------------------------------------------------------
void VCOM_And_HIDTrans_Initiate(S_BUFCTRL* psUart2Usb,S_BUFCTRL* psUsb2Uart);
void VCOM_Serial2Usb(void);

// main ========================================================================================================= 
int main(void)
{	
	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();
	
	// These defines are from  BUFCTRL.h for buffer control in this samle. 
	// Buffer control handler configuration. 
	BUFCTRL_CFG((&g_sUsbToUart),g_i32UsbToUartBuf,sizeof(g_i32UsbToUartBuf)/sizeof(uint32_t));
	BUFCTRL_CFG((&g_sUartToUsb),g_i32UartToUsbBuf,sizeof(g_i32UartToUsbBuf)/sizeof(uint32_t));
	
	// Initiate VCOM, HID transfer and USBD hardware IP.
	VCOM_And_HIDTrans_Initiate((S_BUFCTRL*)(&g_sUsbToUart),(S_BUFCTRL*)(&g_sUartToUsb));
	
	// Process in iterrupt.
	while(1)
	{
		VCOM_Serial2Usb();        
	}
}

void VCOM_Serial2Usb(void) 
{
    UINT32 u32Length = 0;
	UINT8 u8DataBuf[EP2_MAX_PKT_SIZE];
    
    if( !BUFCTRL_IS_EMPTY(g_psVCOM_InBufCtrl) )
	{
        if (g_u8CDC_EP2Empty) 
        {
            g_u8CDC_EP2Empty = 0;
            while( !BUFCTRL_IS_EMPTY(g_psVCOM_InBufCtrl) && u32Length<EP2_MAX_PKT_SIZE )
            {
                BUFCTRL_READ(g_psVCOM_InBufCtrl,(&u8DataBuf[u32Length]));
                u32Length++;
            }
            USBD_MemCopy((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP2)), (uint8_t *)u8DataBuf, u32Length);
            USBD_SET_PAYLOAD_LEN(EP2, u32Length);	 
        }
	}
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

// VCOM and HID transfer ======================================================================================== 
#define VCOM_UART0_PIN_MASK                 (SYS_GPB_MFPH_PB8MFP_Msk|SYS_GPB_MFPH_PB9MFP_Msk)
#define VCOM_UART0_PIN                      (SYS_GPB_MFPH_PB8MFP_UART0_TXD|SYS_GPB_MFPH_PB9MFP_UART0_RXD)

#define VCOM_AND_HIDTRANS_USBD_PIN_MASK     (SYS_GPB_MFPH_PB13MFP_Msk|SYS_GPB_MFPH_PB14MFP_Msk|SYS_GPB_MFPH_PB15MFP_Msk)
#define VCOM_AND_HIDTRANS_USBD_PIN          (SYS_GPB_MFPH_PB13MFP_USBD_DN|SYS_GPB_MFPH_PB14MFP_USBD_DP|SYS_GPB_MFPH_PB15MFP_USBD_VBUS)

UINT32 VCOM_InitUART(S_VCOM_LINECODING* psLineCoding)
{
	// Disable IRQ to config UART.
	NVIC_DisableIRQ(UART0_IRQn);
	// Enable UART module clock 
	CLK_EnableModuleClock(UART0_MODULE);
	// Select UART module clock source as HXT and UART module clock divider as 1
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));
	// Configure UART0 and set UART0 Baud rate
	UART_Open(UART0, psLineCoding->u32DTERate);
	// Enable Interrupt and install the call back function 
	UART_ENABLE_INT(UART0, (UART_INTEN_RDAIEN_Msk /* | UART_INTEN_THREIEN_Msk | UART_INTEN_RXTOIEN_Msk*/));
	
	// Set line configuration.
	UART_SetLine_Config(UART0,
											psLineCoding->u32DTERate,
											((psLineCoding->u8DataBits>=8||psLineCoding->u8DataBits<5)?UART_WORD_LEN_8:(psLineCoding->u8DataBits-5)),
											((psLineCoding->u8ParityType>=5||psLineCoding->u8ParityType==0)?UART_PARITY_NONE:((psLineCoding->u8ParityType*2-1)<<UART_LINE_PBE_Pos)),
											((psLineCoding->u8CharFormat==0)?UART_STOP_BIT_1:UART_STOP_BIT_2));
	// Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8)
	SYS->GPB_MFPH = (SYS->GPB_MFPH&~VCOM_UART0_PIN_MASK)|VCOM_UART0_PIN;	
	// Enable IRQ to start processing.
	NVIC_EnableIRQ(UART0_IRQn);
	return 1;
}
void VCOM_And_HIDTrans_Initiate(S_BUFCTRL* psUart2Usb,S_BUFCTRL* psUsb2Uart)
{
	// {BaudRate,}
	S_VCOM_LINECODING sLineCoding = {115200, 0, 0, 8};
	
	CLK->CLKSEL4 |= BIT24;
	// Enable USBD module clock.
	CLK_EnableModuleClock(USBD_MODULE);
	// Set USBD clock divid
	CLK_SetModuleClock(USBD_MODULE,CLK_CLKSEL4_USBSEL_PLL,CLK_CLKDIV0_USBD(4));	
	// Initiate USBD hardware IP and input HID request for hand-shake.
	USBD_Open(&gsInfo, VCOM_HIDTrans_ClassRequest, NULL);
	// Initiate endpoint configuration of VCOM & HID 
	VCOM_HIDTrans_Init(psUart2Usb,psUsb2Uart,UART0,&sLineCoding,VCOM_InitUART);
	// Enable USB IRQ
	NVIC_EnableIRQ(USBD_IRQn);
	// Start USBD for processing.
	USBD_Start();	
	// gpio multi-function configuration.
	SYS->GPB_MFPH = (SYS->GPB_MFPH&(~VCOM_AND_HIDTRANS_USBD_PIN_MASK))|VCOM_AND_HIDTRANS_USBD_PIN;
}
void UART0_IRQHandler()
{
	VCOM_UARTIRQ();
}
/*** (C) COPYRIGHT 2018 Nuvoton Technology Corp. ***/
