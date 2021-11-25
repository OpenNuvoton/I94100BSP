/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * @brief    Demonstrate how to use FMC CRC32 ISP command to calculate the CRC32 checksum of APROM, LDROM.
 *
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
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

	/* Reset IP */
	SYS_ResetModule(UART0_RST);

	/* Configure UART0 and set UART0 Baudrate */
	UART_Open(UART0, 115200);
}

int main()
{
	uint32_t    u32Data, u32ChkSum;    /* temporary data */

	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	UART0_Init();                      /* Initialize UART0 */

	/*---------------------------------------------------------------------------------------------------------*/
	/* SAMPLE CODE                                                                                             */
	/*---------------------------------------------------------------------------------------------------------*/

	printf("+------------------------------------+\n");
	printf("|    FMC CRC32 Sample Demo           |\n");
	printf("+------------------------------------+\n");

	// Unlock protected register.
	SYS_UnlockReg();
	
	FMC_Open();                        /* Enable FMC ISP function */

	u32Data = FMC_ReadCID();           /* Read company ID. Should be 0xDA. */
	printf("  Company ID ............................ [0x%08x]\n", u32Data);

	u32Data = FMC_ReadPID();           /* Read product ID. */
	printf("  Product ID ............................ [0x%08x]\n", u32Data);

	/* Read User Configuration CONFIG0 */
	printf("  User Config 0 ......................... [0x%08x]\n", FMC_Read(FMC_CONFIG_BASE));
	/* Read User Configuration CONFIG1 */
	printf("  User Config 1 ......................... [0x%08x]\n", FMC_Read(FMC_CONFIG_BASE+4));

	/* Read Data Flash base address */
	printf("  Data Flash Base Address ............... [0x%08x]\n", FMC_ReadDataFlashBaseAddr());

	printf("\nLDROM (0x100000 ~ 0x101200) CRC32 checksum =>  ");

	/*
	 *  Request FMC hardware to run CRC32 caculation on LDROM.
	 */
	u32ChkSum = FMC_GetChkSum(FMC_LDROM_BASE, FMC_LDROM_SIZE);
	if (u32ChkSum == 0xFFFFFFFF)  {
			printf("Failed on calculating LDROM CRC32 checksum!\n");
			goto lexit;                    /* failed */
	}
	printf("0x%x\n", u32ChkSum);       /* print out LDROM CRC32 check sum value */


	printf("\nAPROM bank0 (0x0 ~ 0x40000) CRC32 checksum =>  ");
	/*
	 *  Request FMC hardware to run CRC32 caculation on APROM bank 0.
	 *  Note that FMC CRC32 checksum calculation area must not cross bank boundary.
	 */
	u32ChkSum = FMC_GetChkSum(FMC_APROM_BASE, 0x40000);
	if (u32ChkSum == 0xFFFFFFFF)  {
			printf("Failed on calculating APROM bank0 CRC32 checksum!\n");
			goto lexit;
	}
	printf("0x%x\n", u32ChkSum);       /* print out APROM CRC32 check sum value */

	/*
	 *  Request FMC hardware to run CRC32 caculation on APROM bank 1.
	 */
	printf("\nAPROM bank1 (0x40000 ~ 0x80000) CRC32 checksum =>  ");
	u32ChkSum = FMC_GetChkSum(FMC_APROM_BASE+0x40000, 0x40000);
	if (u32ChkSum == 0xFFFFFFFF)  {
			printf("Failed on calculating APROM bank1 CRC32 checksum!\n");
			goto lexit;
	}
	printf("0x%x\n", u32ChkSum);       /* print out APROM CRC32 check sum value */

	printf("\nFMC CRC32 checksum test done.\n");

lexit:
	FMC_Close();                       /* Disable FMC ISP function */
	SYS_LockReg();                     /* Lock protected registers */

	while (1);
}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
