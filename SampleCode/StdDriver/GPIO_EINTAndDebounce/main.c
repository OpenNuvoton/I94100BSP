/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * @brief    Show the usage of GPIO external interrupt function and de-bounce function.
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "Platform.h"
#include "ConfigSysClk.h"

/**
 * @brief       External INT0 IRQ
 *
 * @param       None
 *
 * @return      None
 *
 * @details     The External INT0 default IRQ, declared in startup .s.
 */
void EINT0_IRQHandler(void)
{

    /* To check if PA.13 external interrupt occurred */
    if(GPIO_GET_INT_FLAG(PA, BIT13)) {
        GPIO_CLR_INT_FLAG(PA, BIT13);
        printf("PA.13 EINT0 occurred.\n");
    }

    /* To check if PA.15 external interrupt occurred */
    if(GPIO_GET_INT_FLAG(PA, BIT15)) {
        GPIO_CLR_INT_FLAG(PA, BIT15);
        printf("PA.15 EINT0 occurred.\n");
    }

}

/**
 * @brief       External INT1 IRQ
 *
 * @param       None
 *
 * @return      None
 *
 * @details     The External INT1 default IRQ, declared in startup .s.
 */
void EINT1_IRQHandler(void)
{

    /* To check if PC.5 external interrupt occurred */
    if(GPIO_GET_INT_FLAG(PC, BIT5)) {
        GPIO_CLR_INT_FLAG(PC, BIT5);
        printf("PC.5 EINT1 occurred.\n");
    }

}

void UART0_Init()
{
	/* Enable UART module clock */
	CLK_EnableModuleClock(UART0_MODULE);

	/* Select UART module clock source as HXT and UART module clock divider as 1 */
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
/* MAIN function                                                                                           */
/*---------------------------------------------------------------------------------------------------------*/
int main(void)
{
	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	/* Init UART0 for printf */
	UART0_Init();

	printf("\n\nCPU @ %d Hz\n", SystemCoreClock);
	printf("+------------------------------------------------------------+\n");
	printf("|    GPIO EINT0/EINT1 Interrupt and De-bounce Sample Code    |\n");
	printf("+------------------------------------------------------------+\n\n");

	/*-----------------------------------------------------------------------------------------------------*/
	/* GPIO External Interrupt Function Test                                                               */
	/*-----------------------------------------------------------------------------------------------------*/
	printf("EINT0(PA.13 and PA.15) and EINT1(PC.5) are used to test interrupt\n");

	/* Set PA multi-function pin for EINT0(PA.13) */
	SYS->GPA_MFPH = (SYS->GPA_MFPH & (~SYS_GPA_MFPH_PA13MFP_Msk)) | SYS_GPA_MFPH_PA13MFP_INT0;

	/* Set PA multi-function pin for EINT0(PA.15) */
	SYS->GPA_MFPH = (SYS->GPA_MFPH & (~SYS_GPA_MFPH_PA15MFP_Msk)) | SYS_GPA_MFPH_PA15MFP_INT0;

	/* Set PC multi-function pin for EINT1(PC.5) */
	SYS->GPC_MFPL = (SYS->GPC_MFPL & (~SYS_GPC_MFPL_PC5MFP_Msk)) | SYS_GPC_MFPL_PC5MFP_INT1;

	/* Configure PA.13 as EINT0 pin and enable interrupt by falling edge trigger */
	GPIO_SetMode(PA, BIT13, GPIO_MODE_QUASI);
	PA13 = 1;
	GPIO_EnableInt(PA, 13, GPIO_INT_FALLING);

	/* Configure PA.15 as EINT0 pin and enable interrupt by rising edge trigger */
	GPIO_SetMode(PA, BIT15, GPIO_MODE_QUASI);
	PA15 = 1;
	GPIO_EnableInt(PA, 15, GPIO_INT_RISING);
	NVIC_EnableIRQ(EINT0_IRQn);

	/* Configure PC.5 as EINT1 pin and enable interrupt by falling and rising edge trigger */
	GPIO_SetMode(PC, BIT5, GPIO_MODE_QUASI);
	PC5 = 1;
	GPIO_EnableInt(PC, 5, GPIO_INT_BOTH_EDGE);
	NVIC_EnableIRQ(EINT1_IRQn);

	/* Enable interrupt de-bounce function and select de-bounce sampling cycle time is 1024 clocks of LIRC clock */
	GPIO_SET_DEBOUNCE_TIME(GPIO_DBCTL_DBCLKSRC_LIRC, GPIO_DBCTL_DBCLKSEL_1024);
	GPIO_ENABLE_DEBOUNCE(PA, BIT13);
	GPIO_ENABLE_DEBOUNCE(PA, BIT15);
	GPIO_ENABLE_DEBOUNCE(PC, BIT5);

	/* Waiting for interrupts */
	while(1);
}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
