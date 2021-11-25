/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * @brief    Trigger EADC by writing EADC_SWTRG register.
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "ConfigSysClk.h"
#include "Platform.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables and constants                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
volatile uint32_t g_u32AdcIntFlag;

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
    g_u32AdcIntFlag = 1;
    EADC_CLR_INT_FLAG(EADC, 0x1);      /* Clear the A/D ADINT0 interrupt flag */
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
    uint8_t  u8Option;
    int32_t  i32ConversionData;

    printf("\n");
    printf("+----------------------------------------------------------------------+\n");
    printf("|                      SWTRG trigger mode test                         |\n");
    printf("+----------------------------------------------------------------------+\n");

		// EADC Clock Initial
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
	
    while(1) {
        printf("Select input mode:\n");
        printf("  [1] Single end input (channel 2 only)\n");
        printf("  Other keys: exit single mode test\n");
        u8Option = getchar();
        if(u8Option == '1') {
			/* Reset EADC module */
			SYS_ResetModule(EADC_RST);

            /* Set input mode as single-end and enable the A/D converter */
            EADC_Open(EADC, EADC_CTL_DIFFEN_SINGLE_END);

            /* Configure the sample module 0 for analog input channel 2 and software trigger source.*/
            EADC_ConfigSampleModule(EADC, 0, EADC_SOFTWARE_TRIGGER, 2);
			
			/* Set sample module 0 external sampling time to 2 */
			EADC_SetExtendSampleTime(EADC, 0, 2);
			
            /* Clear the A/D ADINT0 interrupt flag for safe */
            EADC_CLR_INT_FLAG(EADC, 0x1);

            /* Enable the sample module 0 interrupt.  */
            EADC_ENABLE_INT(EADC, 0x1);//Enable sample module A/D ADINT0 interrupt.
            EADC_ENABLE_SAMPLE_MODULE_INT(EADC, 0, 0x1);//Enable sample module 0 interrupt.
            NVIC_ClearPendingIRQ(EADC0_IRQn);
            NVIC_EnableIRQ(EADC0_IRQn);

            /* Reset the ADC interrupt indicator and trigger sample module 0 to start A/D conversion */
            g_u32AdcIntFlag = 0;
            EADC_START_CONV(EADC, 0x1);

            /* Wait ADC interrupt (g_u32AdcIntFlag will be set at IRQ_Handler function) */
            while(g_u32AdcIntFlag == 0);

            /* Disable the ADINT0 interrupt */
            EADC_DISABLE_INT(EADC, 0x1);

            /* Get the conversion result of the sample module 0 */
            i32ConversionData = EADC_GET_CONV_DATA(EADC, 0);
            printf("Conversion result of channel 2: 0x%X (%d)\n\n", i32ConversionData, i32ConversionData);
        } else
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
