/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * @brief    Implement CRC in CRC-8 mode and get the CRC checksum result.
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "Platform.h"
#include "ConfigSysClk.h"

void UART0_Init(void)
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

	/* Configure UART0 and set UART0 Baudrate */
	UART_Open(UART0, 115200);
}

/*---------------------------------------------------------------------------------------------------------*/
/*  MAIN function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int main(void)
{
	const uint8_t acCRCSrcPattern[] = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39};
	uint32_t i, u32TargetChecksum = 0x58, u32CalChecksum = 0;

	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	/* Init UART0 for printf */
	UART0_Init();

	printf("\n\nCPU @ %d Hz\n", SystemCoreClock);
	printf("+-----------------------------------------+\n");
	printf("|    CRC-8 Polynomial Mode Sample Code    |\n");
	printf("+-----------------------------------------+\n\n");

	printf("# Calculate [0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39] CRC-8 checksum value.\n");
	printf("    - Seed value is 0x5A             \n");
	printf("    - CPU write data length is 8-bit \n");
	printf("    - Checksum complement disable    \n");
	printf("    - Checksum reverse disable       \n");
	printf("    - Write data complement disable  \n");
	printf("    - Write data reverse disable     \n");
	printf("    - Checksum should be 0x%X        \n\n", u32TargetChecksum);

	/* Enable peripheral clock */
	CLK_EnableModuleClock(CRC_MODULE);

	/* Reset CRC module */
	SYS_ResetModule(CRC_RST);

	/* Configure CRC controller for CRC-8 CPU mode */
	CRC_Open(CRC_8, 0, 0x5A, CRC_CPU_WDATA_8);

	/* Start to execute CRC-8 CPU operation */
	for(i = 0; i < sizeof(acCRCSrcPattern); i++) {
			CRC_WRITE_DATA((acCRCSrcPattern[i] & 0xFF));
	}

	/* Get CRC-8 checksum value */
	u32CalChecksum = CRC_GetChecksum();
	printf("CRC checksum is 0x%X ... %s.\n", u32CalChecksum, (u32CalChecksum == u32TargetChecksum) ? "PASS" : "FAIL");

	/* Disable CRC function */
	CLK_DisableModuleClock(CRC_MODULE);

	while(1);
}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
