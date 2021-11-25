/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * @brief    Use ADINT interrupt to do the EADC continuous scan conversion.
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "ConfigSysClk.h"
#include "Platform.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables and constants                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
int16_t  g_i16ConversionData[8];
volatile uint32_t g_u32SAMPLECount;

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void);
void EADC_FunctionTest(void);

/*---------------------------------------------------------------------------------------------------------*/
/* EADC interrupt handler                                                                                  */
/*---------------------------------------------------------------------------------------------------------*/
void EADC0_IRQHandler(void)
{
	uint32_t u32Temp;

	EADC_CLR_INT_FLAG(EADC, 0x1);      /* Clear the A/D ADINT0 interrupt flag */
	
	/* Get the conversion result of the sample module */
	u32Temp = g_u32SAMPLECount++;
	g_i16ConversionData[u32Temp] = EADC_GET_CONV_DATA(EADC, 4);
	u32Temp = g_u32SAMPLECount++;
	g_i16ConversionData[u32Temp] = EADC_GET_CONV_DATA(EADC, 5);
			u32Temp = g_u32SAMPLECount++;
	g_i16ConversionData[u32Temp] = EADC_GET_CONV_DATA(EADC, 6);
			u32Temp = g_u32SAMPLECount++;
	g_i16ConversionData[u32Temp] = EADC_GET_CONV_DATA(EADC, 7);
}

void UART0_Init()
{
	/* Enable peripheral clock */
	CLK_EnableModuleClock(UART0_MODULE);

	/* Peripheral clock source */
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));

	/* Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8) */
	SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk);
	SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD);

	/* Reset UART module */
	SYS_ResetModule(UART0_RST);

	/* Configure UART0 and set UART0 baud rate */
	UART_Open(UART0, 115200);
}

/*---------------------------------------------------------------------------------------------------------*/
/* EADC function test                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void EADC_FunctionTest()
{
    uint8_t  u8Option, u8SAMPLECount = 0;
    
	// EADC Initial.
	{
		/* Enable EADC module clock */
		CLK_EnableModuleClock(EADC_MODULE);

		/* EADC clock source is PCLK1 = 72MHz/2, set divider to 8, EADC clock is 36/8 MHz */
		CLK_SetModuleClock(EADC_MODULE, 0, CLK_CLKDIV0_EADC(8));

		/* Configure the GPA0 - GPA3 ADC analog input pins.  */
		SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA0MFP_Msk | SYS_GPA_MFPL_PA1MFP_Msk |
											 SYS_GPA_MFPL_PA2MFP_Msk | SYS_GPA_MFPL_PA3MFP_Msk);
		SYS->GPA_MFPL |= (SYS_GPA_MFPL_PA0MFP_EADC0_CH0 | SYS_GPA_MFPL_PA1MFP_EADC0_CH1 |
											SYS_GPA_MFPL_PA2MFP_EADC0_CH2 | SYS_GPA_MFPL_PA3MFP_EADC0_CH3);

		/* Disable the GPA0 - GPA3 digital input path to avoid the leakage current. */
		GPIO_DISABLE_DIGITAL_PATH(PA, 0xF);
	}

    printf("\n");
    printf("+----------------------------------------------------------------------+\n");
    printf("|                      ADINT trigger mode test                         |\n");
    printf("+----------------------------------------------------------------------+\n");

    printf("\nIn this test, software will get 2 cycles of conversion result from the specified channels.\n");

    while(1) {
        printf("\n\nSelect input mode:\n");
        printf("  [1] Single end input (channel 0, 1, 2 and 3)\n");
        printf("  Other keys: exit continuous scan mode test\n");
        u8Option = getchar();
		memset(g_i16ConversionData, 0, sizeof(g_i16ConversionData));
		
        if(u8Option == '1') {
			/* Reset EADC module */
			SYS_ResetModule(EADC_RST);
			
            /* Set input mode as single-end and enable the A/D converter */
            EADC_Open(EADC, EADC_CTL_DIFFEN_SINGLE_END);

            /* Configure the sample 4 module for analog input channel 0 and enable ADINT0 trigger source */
            EADC_ConfigSampleModule(EADC, 4, EADC_ADINT0_TRIGGER, 0);
			/* Set sample module 4 external sampling time to 2 */
            EADC_SetExtendSampleTime(EADC, 4, 2);
			/* Configure the sample 5 module for analog input channel 1 and enable ADINT0 trigger source */
            EADC_ConfigSampleModule(EADC, 5, EADC_ADINT0_TRIGGER, 1);
			/* Set sample module 5 external sampling time to 2 */
            EADC_SetExtendSampleTime(EADC, 5, 2);
			/* Configure the sample 6 module for analog input channel 2 and enable ADINT0 trigger source */
            EADC_ConfigSampleModule(EADC, 6, EADC_ADINT0_TRIGGER, 2);
			/* Set sample module 6 external sampling time to 2 */
            EADC_SetExtendSampleTime(EADC, 6, 2);
			/* Configure the sample 7 module for analog input channel 3 and enable ADINT0 trigger source */
            EADC_ConfigSampleModule(EADC, 7, EADC_ADINT0_TRIGGER, 3);
			/* Set sample module 7 external sampling time to 2 */
			EADC_SetExtendSampleTime(EADC, 7, 2);
			
            /* Clear the A/D ADINT0 interrupt flag for safe */
            EADC_CLR_INT_FLAG(EADC, 0x1);

            /* Enable the sample module 7 interrupt */
            EADC_ENABLE_INT(EADC, 0x1);//Enable sample module  A/D ADINT0 interrupt.
            EADC_ENABLE_SAMPLE_MODULE_INT(EADC, 0, (0x1 << 7));//Enable sample module 7 interrupt.
			NVIC_ClearPendingIRQ(EADC0_IRQn);
            NVIC_EnableIRQ(EADC0_IRQn);

            /* Reset the ADC indicator and trigger sample module 7 to start A/D conversion */
            g_u32SAMPLECount = 0;
			EADC_START_CONV(EADC, (0xf << 4));

            /* Wait EADC interrupt (g_u32SAMPLECount will be set at IRQ_Handler function) */
            while(g_u32SAMPLECount < 8);

            /* Disable the sample module 7 interrupt */
            EADC_DISABLE_SAMPLE_MODULE_INT(EADC, 0, (0x1 << 7));
         
            for(u8SAMPLECount = 0; (u8SAMPLECount) < 8; u8SAMPLECount++)
						printf("Conversion result of channel %d: 0x%X (%d)\n", (u8SAMPLECount<4)?u8SAMPLECount:(u8SAMPLECount%4), g_i16ConversionData[u8SAMPLECount], g_i16ConversionData[u8SAMPLECount]);

        } 
				else
            return ;
    }
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	/* Init UART0 for printf */
	UART0_Init();

	/*---------------------------------------------------------------------------------------------------------*/
	/* SAMPLE CODE                                                                                             */
	/*---------------------------------------------------------------------------------------------------------*/

	printf("\nSystem clock rate: %d Hz", SystemCoreClock);

	/* EADC function test */
	EADC_FunctionTest();

	/* Disable EADC IP clock */
	CLK_DisableModuleClock(EADC_MODULE);

	/* Disable External Interrupt */
	NVIC_DisableIRQ(EADC0_IRQn);

	printf("Exit EADC sample code\n");

	while(1);
}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
