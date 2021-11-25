/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * @brief    Demonstrate how to trigger EADC by PWM.
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "ConfigSysClk.h"
#include "Platform.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables and constants                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
volatile uint32_t g_u32AdcIntFlag, g_u32COVNUMFlag = 0;

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
    EADC_CLR_INT_FLAG(EADC, 0x1);      /* Clear the A/D ADINT0 interrupt flag */
    g_u32AdcIntFlag = 1;
    g_u32COVNUMFlag++;
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

void PWM0_Init()
{
	/*---------------------------------------------------------------------------------------------------------*/
	/* Init PWM0                                                                                               */
	/*---------------------------------------------------------------------------------------------------------*/
	/* Enable PWM0 module clock */
	CLK_EnableModuleClock(PWM0_MODULE);

	/* Select PWM0 module clock source as PCLK0 */
	CLK_SetModuleClock(PWM0_MODULE, CLK_CLKSEL2_PWM0SEL_PCLK0, 0);

	/* Set PB multi-function pins for PWM0 Channel0 */
	SYS->GPB_MFPL = (SYS->GPB_MFPL & (~SYS_GPB_MFPL_PB2MFP_Msk));
	SYS->GPB_MFPL |= SYS_GPB_MFPL_PB2MFP_PWM0_CH0;

	/* Reset PWM0 module */
	SYS_ResetModule(PWM0_RST);

	/* Set PWM0 timer clock prescaler */
	PWM_SET_PRESCALER(PWM0, 0, 255);

	/* Set up counter type */
	PWM0->CTL1 &= ~PWM_CTL1_CNTTYPE0_Msk;

	/* Set PWM0 timer duty */
	PWM_SET_CMR(PWM0, 0, 32767);

	/* Set PWM0 timer period */
	PWM_SET_CNR(PWM0, 0, 65535);

	/* PWM period point trigger ADC enable */
	PWM_EnableADCTrigger(PWM0, 0, PWM_TRIGGER_ADC_EVEN_PERIOD_POINT);

	/* Set output level at zero, compare up, period(center) and compare down of specified channel */
	PWM_SET_OUTPUT_LEVEL(PWM0, 0x1, PWM_OUTPUT_HIGH, PWM_OUTPUT_LOW, PWM_OUTPUT_NOTHING, PWM_OUTPUT_NOTHING);

	/* Enable output of PWM0 channel 0 */
	PWM_EnableOutput(PWM0, 0x1);
}

/*---------------------------------------------------------------------------------------------------------*/
/* EADC function test                                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
void EADC_FunctionTest()
{
    uint8_t  u8Option;
    int32_t  i32ConversionData[6] = {0};

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
    printf("|                       PWM trigger mode test                          |\n");
    printf("+----------------------------------------------------------------------+\n");

    printf("\nIn this test, software will get 6 conversion result from the specified channel.\n");

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

            /* Configure the sample module 0 for analog input channel 2 and enable PWM0 trigger source */
            EADC_ConfigSampleModule(EADC, 0, EADC_PWM0TG0_TRIGGER, 2);
			
			/* Set sample module 0 external sampling time to 2 */
			EADC_SetExtendSampleTime(EADC, 0, 2);
			
            /* Clear the A/D ADINT0 interrupt flag for safe */
            EADC_CLR_INT_FLAG(EADC, 0x1);

            /* Enable the sample module 0 interrupt */
            EADC_ENABLE_INT(EADC, 0x1);//Enable sample module A/D ADINT0 interrupt.
            EADC_ENABLE_SAMPLE_MODULE_INT(EADC, 0, 0x1);//Enable sample module 0 interrupt.
			NVIC_ClearPendingIRQ(EADC0_IRQn);
            NVIC_EnableIRQ(EADC0_IRQn);

            printf("Conversion result of channel 2:\n");

            /* Reset the EADC indicator and enable PWM0 channel 0 counter */
            g_u32AdcIntFlag = 0;
            g_u32COVNUMFlag = 0;
            PWM_Start(PWM0, 0x1); //PWM0 channel 0 counter start running.

            while(1) {
                /* Wait ADC interrupt (g_u32AdcIntFlag will be set at IRQ_Handler function) */
                while(g_u32AdcIntFlag == 0);

                /* Reset the ADC interrupt indicator */
                g_u32AdcIntFlag = 0;

                /* Get the conversion result of the sample module 0 */
                i32ConversionData[g_u32COVNUMFlag - 1] = EADC_GET_CONV_DATA(EADC, 0);

                if(g_u32COVNUMFlag > 6)
                    break;
            }

            /* Disable PWM0 channel 0 counter */
            PWM_ForceStop(PWM0, 0x1); //PWM0 counter stop running.

            /* Disable sample module 0 interrupt */
            EADC_DISABLE_SAMPLE_MODULE_INT(EADC, 0, 0x1);

            for(g_u32COVNUMFlag = 0; (g_u32COVNUMFlag) < 6; g_u32COVNUMFlag++)
                printf("                                0x%X (%d)\n", i32ConversionData[g_u32COVNUMFlag], i32ConversionData[g_u32COVNUMFlag]);

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

	/* Init PWM for EADC */
	PWM0_Init();

	/*---------------------------------------------------------------------------------------------------------*/
	/* SAMPLE CODE                                                                                             */
	/*---------------------------------------------------------------------------------------------------------*/

	printf("\nSystem clock rate: %d Hz", SystemCoreClock);

	/* EADC function test */
	EADC_FunctionTest();

	/* Disable EADC IP clock */
	CLK_DisableModuleClock(EADC_MODULE);

	/* Disable PWM0 IP clock */
	CLK_DisableModuleClock(PWM0_MODULE);

	/* Disable External Interrupt */
	NVIC_DisableIRQ(EADC0_IRQn);

	printf("Exit EADC sample code\n");

	while(1);
}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
