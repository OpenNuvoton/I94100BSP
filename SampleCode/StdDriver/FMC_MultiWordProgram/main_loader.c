/******************************************************************************
 * @file     main_loader.c
 * @version  V1.00
 * @brief    Load multi_word_prog.bin image to SRAM and branch to execute it.
 *
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include <string.h>
#include "ConfigSysClk.h"

#include "Platform.h"

#define SRAM_IMAGE_BASE             0x20004000   /* The execution address of multi_word_prog.bin. */

typedef void (FUNC_PTR)(void);

extern uint32_t  loaderImage1Base, loaderImage1Limit;   /* symbol of image start and end */

void UART0_Init(void)
{
	/* Enable peripheral clock */
	CLK_EnableModuleClock(UART0_MODULE);

	/* Peripheral clock source */
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));

	/* Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8) */
	SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk);
	SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD);

	/* Reset IP */
	SYS_ResetModule(UART0_RST);

	/* Configure UART0 and set UART0 Baudrate */
	UART_Open(UART0, 115200);
}

int main()
{
    FUNC_PTR    *func;                 /* function pointer */

		// Initiate system clock(Configure in ConfigSysClk.h)
		SYSCLK_INITIATE();
	
    UART0_Init();                      /* Initialize UART0 */

    /*---------------------------------------------------------------------------------------------------------*/
    /* SAMPLE CODE                                                                                             */
    /*---------------------------------------------------------------------------------------------------------*/

    printf("\n\n");
    printf("+---------------------------------------------+\n");
    printf("|    FMC_MultiWordProgram Sample Loader       |\n");
    printf("+---------------------------------------------+\n");

		/* Unlock protected registers */
		SYS_UnlockReg();

		FMC_Open();                        /* Enable FMC ISP function */

    /* Load multi_word_prog.bin image to SRAM address 0x4000. */
    memcpy((uint8_t *)SRAM_IMAGE_BASE, (uint8_t *)&loaderImage1Base, (uint32_t)&loaderImage1Limit - (uint32_t)&loaderImage1Base);

    /* Get the Reset_Handler entry address of multi_word_prog.bin. */
    func = (FUNC_PTR *)*(uint32_t *)(SRAM_IMAGE_BASE+4);

    /* Get and set the SP (Stack Pointer Base) of multi_word_prog.bin. */
	__set_MSP(*(uint32_t *)SRAM_IMAGE_BASE);
    /*
     *  Brach to the multi_word_prog.bin's reset handler in way of function call.
     */
    func();

    SYS_LockReg();                     /* Lock protected registers */

    while (1);
}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
