/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * @brief    Demonstrate how to trigger EADC by PWM and transfer conversion data by PDMA.
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "Platform.h"
#include "ConfigSysClk.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables and constants                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
volatile uint32_t g_u32AdcIntFlag, g_u32COVNUMFlag = 0;
volatile uint32_t g_u32IsTestOver = 0;
int16_t  g_i32ConversionData[6] = {0};
uint32_t g_u32SampleModuleNum = 0;

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void);
void EADC_FunctionTest(void);

/*---------------------------------------------------------------------------------------------------------*/
/* PDMA interrupt handler                                                                                  */
/*---------------------------------------------------------------------------------------------------------*/
void PDMA_IRQHandler(void)
{
    uint32_t status = PDMA_GET_INT_STATUS();

    if(status & 0x1) {  /* abort */
        if(PDMA_GET_ABORT_STS() & 0x4)
            g_u32IsTestOver = 2;
        PDMA_CLR_ABORT_FLAG(PDMA_CH2_MASK);
    } else if(status & 0x2) {  /* done */
        if(PDMA_GET_TD_STS() & 0x4)
            g_u32IsTestOver = 1;
        PDMA_CLR_TD_FLAG(PDMA_CH2_MASK);
    } else
        printf("unknown interrupt !!\n");
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

	/* Reset PWM0 module */
	SYS_ResetModule(PWM0_RST);

	/* Set PWM0 timer clock prescaler */
	PWM_SET_PRESCALER(PWM0, 0, 0);

	/* Set up counter type */
	PWM0->CTL1 &= ~PWM_CTL1_CNTTYPE0_Msk;

	/* Set PWM0 timer duty */
	PWM_SET_CMR(PWM0, 0, 108);

	/* Set PWM0 timer period */
	PWM_SET_CNR(PWM0, 0, 216);

	/* PWM period point trigger ADC enable */
	PWM_EnableADCTrigger(PWM0, 0, PWM_TRIGGER_ADC_EVEN_PERIOD_POINT);

	/* Set output level at zero, compare up, period(center) and compare down of specified channel */
	PWM_SET_OUTPUT_LEVEL(PWM0, 0x1, PWM_OUTPUT_HIGH, PWM_OUTPUT_LOW, PWM_OUTPUT_NOTHING, PWM_OUTPUT_NOTHING);

	/* Enable output of PWM0 channel 0 */
	PWM_EnableOutput(PWM0, 0x1);
}

void PDMA_Init()
{
	/*---------------------------------------------------------------------------------------------------------*/
	/* Init PDMA                                                                                               */
	/*---------------------------------------------------------------------------------------------------------*/
	/* Enable PDMA clock source */
	CLK_EnableModuleClock(PDMA_MODULE);

	/* Reset PDMA module */
	SYS_ResetModule(PDMA_RST);

	/* Configure PDMA peripheral mode form EADC to memory */
	/* Open Channel 2 */
	PDMA_Open(0x4);

	/* transfer width is half word(16 bit) and transfer count is 6 */
	PDMA_SetTransferCnt(2, PDMA_WIDTH_16, 6);

	/* Set source address as EADC data register(no increment) and destination address as g_i32ConversionData array(increment) */
	PDMA_SetTransferAddr(2, (uint32_t)&EADC->DAT[g_u32SampleModuleNum], PDMA_SAR_FIX, (uint32_t)g_i32ConversionData, PDMA_DAR_INC);

	/* Select PDMA request source as ADC RX */
	PDMA_SetTransferMode(2, PDMA_EADC_RX, FALSE, 0);

	/* Set PDMA as single request type for EADC */
	PDMA_SetBurstType(2, PDMA_REQ_SINGLE, PDMA_BURST_4);

	PDMA_EnableInt(2, PDMA_INT_TRANS_DONE);
	NVIC_ClearPendingIRQ(PDMA_IRQn);
	NVIC_EnableIRQ(PDMA_IRQn);
}

void ReloadPDMA()
{
    /* transfer width is half word(16 bit) and transfer count is 6 */
    PDMA_SetTransferCnt(2, PDMA_WIDTH_16, 6);

    /* Select PDMA request source as ADC RX */
    PDMA_SetTransferMode(2, PDMA_EADC_RX, FALSE, 0);
}

/*---------------------------------------------------------------------------------------------------------*/
/* EADC function test                                                                                      */
/*---------------------------------------------------------------------------------------------------------*/
void EADC_FunctionTest()
{
    uint8_t  u8Option;

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
	
    printf("\n");
    printf("+----------------------------------------------------------------------+\n");
    printf("|     PWM trigger mode and transfer conversion data by PDMA test       |\n");
    printf("+----------------------------------------------------------------------+\n");

    printf("\nIn this test, software will get 6 conversion result from the specified channel.\n");

    while(1) {
        /* reload PDMA configuration for next transmission */
        ReloadPDMA();

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
            EADC_ConfigSampleModule(EADC, g_u32SampleModuleNum, EADC_PWM0TG0_TRIGGER, 2);
			/* Set sample module g_u32SampleModuleNum external sampling time to 2 */
			EADC_SetExtendSampleTime(EADC, g_u32SampleModuleNum, 2);
			
            EADC_ENABLE_PDMA(EADC);

            printf("Conversion result of channel 2:\n");

            /* Enable PWM0 channel 0 counter */
            PWM_Start(PWM0, 0x1); /* PWM0 channel 0 counter start running. */

            while(1) {
                /* Wait PDMA interrupt (g_u32IsTestOver will be set at IRQ_Handler function) */
                while(g_u32IsTestOver == 0);
                break;
            }
            g_u32IsTestOver = 0;

            /* Disable PWM0 channel 0 counter */
            PWM_ForceStop(PWM0, 0x1); /* PWM0 counter stop running. */

            for(g_u32COVNUMFlag = 0; (g_u32COVNUMFlag) < 6; g_u32COVNUMFlag++)
                printf("                                0x%X (%d)\n", g_i32ConversionData[g_u32COVNUMFlag], g_i32ConversionData[g_u32COVNUMFlag]);

        } 
				else
            return ;

        EADC_Close(EADC);
        /* Reset EADC module */
        SYS_ResetModule(EADC_RST);
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
	/* Init PWM for EADC */
	PWM0_Init();

	/* Init PDMA for EADC */
	PDMA_Init();

	printf("\nSystem clock rate: %d Hz", SystemCoreClock);

	/* EADC function test */
	EADC_FunctionTest();

	/* Disable EADC IP clock */
	CLK_DisableModuleClock(EADC_MODULE);

	/* Disable PWM0 IP clock */
	CLK_DisableModuleClock(PWM0_MODULE);

	/* Disable PDMA clock source */
	CLK_DisableModuleClock(PDMA_MODULE);

	/* Disable PDMA Interrupt */
	NVIC_DisableIRQ(PDMA_IRQn);

	printf("Exit EADC sample code\n");

	while(1);
}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
