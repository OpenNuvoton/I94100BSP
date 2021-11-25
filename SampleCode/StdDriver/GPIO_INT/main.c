/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * @brief    Show the usage of GPIO interrupt function.
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "Platform.h"
#include "ConfigSysClk.h"

/**
 * @brief       GPIO PB IRQ
 *
 * @param       None
 *
 * @return      None
 *
 * @details     The PB default IRQ, declared in startup .s.
 */
void GPB_IRQHandler(void)
{
    /* To check if PB.2 interrupt occurred */
    if(GPIO_GET_INT_FLAG(PB, BIT2)) {
        GPIO_CLR_INT_FLAG(PB, BIT2);
        printf("PB.2 INT occurred.\n");
    } else {
        /* Un-expected interrupt. Just clear all PB interrupts */
        PB->INTSRC = PB->INTSRC;
        printf("Un-expected interrupts.\n");
    }
}

/**
 * @brief       GPIO PC IRQ
 *
 * @param       None
 *
 * @return      None
 *
 * @details     The PC default IRQ, declared in startup .s.
 */
void GPC_IRQHandler(void)
{
    /* To check if PC.5 interrupt occurred */
    if(GPIO_GET_INT_FLAG(PC, BIT5)) {
        GPIO_CLR_INT_FLAG(PC, BIT5);
        printf("PC.5 INT occurred.\n");
    } else {
        /* Un-expected interrupt. Just clear all PC interrupts */
        PC->INTSRC = PC->INTSRC;
        printf("Un-expected interrupts.\n");
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
	printf("+------------------------------------------------+\n");
	printf("|    GPIO PB.2 and PC.5 Interrupt Sample Code    |\n");
	printf("+------------------------------------------------+\n\n");

	/*-----------------------------------------------------------------------------------------------------*/
	/* GPIO Interrupt Function Test                                                                        */
	/*-----------------------------------------------------------------------------------------------------*/
	printf("PB.2 and PC.5 are used to test interrupt ......\n");

	/* Configure PB.2 as Quasi-bidirection mode and enable interrupt by rising edge trigger */
	GPIO_SetMode(PB, BIT2, GPIO_MODE_QUASI);
	PB2 = 1;
	GPIO_EnableInt(PB, 2, GPIO_INT_RISING);
	NVIC_EnableIRQ(GPB_IRQn);

	/* Configure PC.5 as Quasi-bidirection mode and enable interrupt by falling edge trigger */
	GPIO_SetMode(PC, BIT5, GPIO_MODE_QUASI);
	PC5 = 1;
	GPIO_EnableInt(PC, 5, GPIO_INT_FALLING);
	NVIC_EnableIRQ(GPC_IRQn);

	/* Enable interrupt de-bounce function and select de-bounce sampling cycle time is 1024 clocks of LIRC clock */
	GPIO_SET_DEBOUNCE_TIME(GPIO_DBCTL_DBCLKSRC_LIRC, GPIO_DBCTL_DBCLKSEL_1024);
	GPIO_ENABLE_DEBOUNCE(PB, BIT2);
	GPIO_ENABLE_DEBOUNCE(PC, BIT5);

	/* Waiting for interrupts */
	while(1);
}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
