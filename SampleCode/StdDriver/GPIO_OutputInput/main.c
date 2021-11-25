/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * @brief    Show how to set GPIO pin mode and use pin data input/output control.
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "stdio.h"
#include "Platform.h"
#include "ConfigSysClk.h"

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
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{

	int32_t i32Err, i32TimeOutCnt;

	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	/* Init UART0 for printf */
	UART0_Init();

	printf("\n\nCPU @ %dHz\n", SystemCoreClock);

	printf("+-------------------------------------------------+\n");
	printf("|    PB.3(Output) and PD.7(Input) Sample Code     |\n");
	printf("+-------------------------------------------------+\n\n");

	/*-----------------------------------------------------------------------------------------------------*/
	/* GPIO Basic Mode Test --- Use Pin Data Input/Output to control GPIO pin                              */
	/*-----------------------------------------------------------------------------------------------------*/
	printf("  >> Please connect PB.3 and PD.7 first << \n");
	printf("     Press any key to start test by using [Pin Data Input/Output Control] \n\n");
	getchar();

	/* Configure PB.3 as Output mode and PD.7 as Input mode then close it */
	GPIO_SetMode(PB, BIT3, GPIO_MODE_OUTPUT);
	GPIO_SetMode(PD, BIT7, GPIO_MODE_INPUT);

	i32Err = 0;
	printf("GPIO PB.3(output mode) connect to PD.7(input mode) ......");

	/* Use Pin Data Input/Output Control to pull specified I/O or get I/O pin status */
	/* Set PB.3 output pin value is low */
	PB3 = 0;

	/* Set time out counter */
	i32TimeOutCnt = 100;

	/* Wait for PD.7 input pin status is low for a while */
	while(PD7 != 0) {
			if(i32TimeOutCnt > 0) {
					i32TimeOutCnt--;
			} else {
					i32Err = 1;
					break;
			}
	}

	/* Set PB.3 output pin value is high */
	PB3 = 1;

	/* Set time out counter */
	i32TimeOutCnt = 100;

	/* Wait for PD.7 input pin status is high for a while */
	while(PD7 != 1) {
			if(i32TimeOutCnt > 0) {
					i32TimeOutCnt--;
			} else {
					i32Err = 1;
					break;
			}
	}

	/* Print test result */
	if(i32Err) {
			printf("  [FAIL].\n");
	} else {
			printf("  [OK].\n");
	}

	/* Configure PB.3 and PD.7 to default Quasi-bidirectional mode */
	GPIO_SetMode(PB, BIT3, GPIO_MODE_QUASI);
	GPIO_SetMode(PD, BIT7, GPIO_MODE_QUASI);

	while(1);
}
