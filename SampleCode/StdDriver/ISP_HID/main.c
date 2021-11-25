/******************************************************************************
 * @file     main.c
 * @brief
 *           Demonstrate how to transfer data between USB device and PC through USB HID interface.
 *           A windows tool is also included in this sample code to connect with USB device.
 *
 * @note
 * Copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "Platform.h"
#include "targetdev.h"

#define CLK_PLLCTL_SRC_HIRC48M_96M   (CLK_PLLCTL_PLLSRC_HIRC | CLK_PLLCTL_NR(6) | CLK_PLLCTL_NF(24) | CLK_PLLCTL_NO_4)
#define PLL_CLOCK       		96000000
#define USE_PD2_DETECT          1
#define HCLK_DIV 						2
#define USBD_DIV 						2

#define USBD_HID_PIN_MASK (SYS_GPB_MFPH_PB13MFP_Msk|SYS_GPB_MFPH_PB14MFP_Msk|SYS_GPB_MFPH_PB15MFP_Msk)
#define USBD_HID_PIN      (SYS_GPB_MFPH_PB13MFP_USBD_DN|SYS_GPB_MFPH_PB14MFP_USBD_DP|SYS_GPB_MFPH_PB15MFP_USBD_VBUS)

/*--------------------------------------------------------------------------*/
void SYS_Init(void)
{
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init System Clock                                                                                       */
    /*---------------------------------------------------------------------------------------------------------*/
    /* Enable Internal RC clock clock */
    CLK->PWRCTL |= CLK_PWRCTL_HIRCEN_Msk;

    /* Set core clock as PLL_CLOCK from PLL */
    CLK->PLLCTL = CLK_PLLCTL_SRC_HIRC48M_96M;

    while (!(CLK->STATUS & CLK_STATUS_PLLSTB_Msk));

		CLK->CLKDIV0 = (CLK->CLKDIV0 & ~(CLK_CLKDIV0_HCLKDIV_Msk | CLK_CLKDIV0_USBDIV_Msk)) | (CLK_CLKDIV0_HCLK(HCLK_DIV) | CLK_CLKDIV0_USBD(USBD_DIV));
		CLK->CLKSEL0 = (CLK->CLKSEL0 & ~(CLK_CLKSEL0_HCLKSEL_Msk | CLK_CLKSEL0_HIRCFSEL_Msk)) | (CLK_CLKSEL0_HCLKSEL_PLL | CLK_CLKSEL0_HIRCFSEL_48M);
		CLK->CLKSEL4 = (CLK->CLKSEL4 & ~(CLK_CLKSEL4_USBSEL_Msk)) | CLK_CLKSEL4_USBSEL_PLL;
		
    /* Update System Core Clock */
    /* User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CycylesPerUs automatically. */
	
    /* Enable module clock */
    CLK->APBCLK0 |= CLK_APBCLK0_USBDCKEN_Msk;
    CLK->AHBCLK |= CLK_AHBCLK_ISPCKEN_Msk;
    /*---------------------------------------------------------------------------------------------------------*/
    /* Init I/O Multi-function                                                                                 */
    /*---------------------------------------------------------------------------------------------------------*/
    // gpio multi-function configuration.
    SYS->GPB_MFPH = (SYS->GPB_MFPH & (~USBD_HID_PIN_MASK)) | USBD_HID_PIN;
}

volatile uint32_t isp_delay=0x1ffff;
/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
	uint8_t *ptr;
    
  /* Unlock write-protected registers */
	SYS_UnlockReg();
	/* Init system and multi-funcition I/O */
	SYS_Init();
	FMC->ISPCTL |= FMC_ISPCTL_ISPEN_Msk;	// (1ul << 0)
	GetDataFlashInfo(&g_dataFlashAddr, &g_dataFlashSize);

	while (DetectPin == 0)
	{
		/* Open USB controller */
		USBD_Open(&gsInfo, HIDTrans_ClassRequest, NULL);
		/*Init Endpoint configuration for HID */
		HID_Init();
		// Enable USB IRQ
		NVIC_EnableIRQ(USBD_IRQn);
		/* Start USB device */
		USBD_Start();

		while (DetectPin == 0)
		{
			if (bUsbDataReady == TRUE) 
			{
				ParseCmd((uint8_t *)usb_rcvbuf, 64);
				ptr = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP2));
				/* Prepare the data for next HID IN transfer */
				USBD_MemCopy(ptr, response_buff, EP2_MAX_PKT_SIZE);
				USBD_SET_PAYLOAD_LEN(EP2, EP2_MAX_PKT_SIZE);
				bUsbDataReady = FALSE;
			}
		}

		goto _APROM;
   }

		/* Waiting for down-count to zero */
		while(isp_delay-- >0);

_APROM:
	outpw(&SYS->RSTSTS, 3);//clear bit
	outpw(&FMC->ISPCTL, inpw(&FMC->ISPCTL) & 0xFFFFFFFC);
	outpw(&SCB->AIRCR, (V6M_AIRCR_VECTKEY_DATA | V6M_AIRCR_SYSRESETREQ));

	/* Trap the CPU */
	while (1);
}
/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
