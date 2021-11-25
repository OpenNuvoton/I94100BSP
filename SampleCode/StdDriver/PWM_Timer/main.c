/**************************************************************************//**
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 20/02/11 10:04a $
 * @brief
 *           
 *
 * @note
 * Copyright (C) 2020 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "Platform.h"
#include "ConfigSysClk.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Macro, type and constant definitions                                                                    */
/*---------------------------------------------------------------------------------------------------------*/

void UART0_Init(void);

uint8_t u8Flag0;

/**
 * @brief       PWM0 IRQ Handler
 *
 * @param       None
 *
 * @return      None
 *
 * @details     ISR to handle PWM0 interrupt event
 */
void PWM0P0_IRQHandler(void)
{
		if(PWM_GetZeroIntFlag(PWM0, 0))
		{
			u8Flag0 = 1; 
			PWM_ClearZeroIntFlag(PWM0, 0);
		}
}


void PWM_Init()
{
	/* Enable PWM0 module clock */
	CLK_EnableModuleClock(PWM0_MODULE);

	/*---------------------------------------------------------------------------------------------------------*/
	/* PWM clock frequency configuration                                                                       */
	/*---------------------------------------------------------------------------------------------------------*/
	/* PWM clock frequency can be set equal or double to HCLK by choosing case 1 or case 2 */
	/* case 1.PWM clock frequency is set equal to HCLK: select PWM module clock source as PCLK */
	CLK_SetModuleClock(PWM0_MODULE, CLK_CLKSEL2_PWM0SEL_PCLK0, NULL);

	/* case 2.PWM clock frequency is set double to HCLK: select PWM module clock source as PLL */
	//CLK_SetModuleClock(PWM0_MODULE, CLK_CLKSEL2_PWM0SEL_PLL, NULL);
	/*---------------------------------------------------------------------------------------------------------*/

	/* Reset PWM0 module */
	SYS_ResetModule(PWM0_RST);
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
	uint32_t u32Tick;
	
	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	/* Init UART to 115200-8n1 for print message */
	UART0_Init();

	// PWM Initial
	PWM_Init();

	printf("\n\nCPU @ %dHz(PLL@ %dHz)\n", SystemCoreClock, PllClock);
	printf("PWM0 clock is from %s\n", (CLK->CLKSEL2 & CLK_CLKSEL2_PWM0SEL_Msk) ? "CPU" : "PLL");
	printf("+------------------------------------------------------------------------+\n");
	printf("|                          PWM Timer Sample Code                         |\n");
	printf("|                                                                        |\n");
	printf("+------------------------------------------------------------------------+\n");
	
	// Down counter
	// Auto-reload
	// Set the PWM frequency at 1Hz
	PWM_ConfigOutputChannel(PWM0, 0, 1, 0);
	
	// Channel 0 Independent mode
	PWM_DISABLE_COMPLEMENTARY_CHAN_MODE(PWM0, PWM_CHAN0_COMP_OUTPUT);
		
	// Disable output of PWM0 Channel0
	PWM_DisableOutput(PWM0, PWM_CH_0_MASK);

	// Disable other interrupt source.
	PWM_DisablePeriodInt(PWM0, 0); 
	PWM_DisableDutyInt(PWM0, 0);
	
	// Enable PWM0 channel 0 zero point interrupt
	PWM_ClearZeroIntFlag(PWM0, 0);
	PWM_EnableZeroInt(PWM0, 0);
	NVIC_EnableIRQ(PWM0_P0_IRQn);

	// Start count
	u8Flag0 = 0;
	u32Tick = 0;
	PWM_Start(PWM0, PWM_CH_0_MASK);
	printf("%d sec\n", u32Tick++);

	while(1)
	{
		if(u8Flag0 == 1)
		{
			printf("%d sec\n", u32Tick++);
			u8Flag0 = 0;
		}
	}
}


void UART0_Init(void)
{
	/* Enable UART module clock */
	CLK_EnableModuleClock(UART0_MODULE);

	/* Select UART module clock source as PLL and UART module clock divider as 1 */
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));

	/*---------------------------------------------------------------------------------------------------------*/
	/* Init I/O Multi-function                                                                                 */
	/*---------------------------------------------------------------------------------------------------------*/
	/* Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8) */
	SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk)) | (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD);

	/* Reset UART module */
	SYS_ResetModule(UART0_RST);

	/* Configure UART0 and set UART0 baud rate */
	UART_Open(UART0, 115200);
}
