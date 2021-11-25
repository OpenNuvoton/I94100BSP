/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * @brief    Show FMC read Flash IDs, erase, read, and write function
 *
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "ConfigSysClk.h"
#include "Platform.h"

#define APROM_TEST_BASE             0x20000      /* APROM test start address                 */
#define DATA_FLASH_TEST_BASE        0x7F000      /* Data Flash test start address            */
#define DATA_FLASH_TEST_END         0x80000      /* Data Flash test end address              */
#define TEST_PATTERN                0x124836C9   /* Test pattern                             */

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

static int  set_data_flash_base(uint32_t u32DFBA)
{
    uint32_t   au32Config[3];          /* User Configuration */

    /* Read User Configuration 0~2 */
    if (FMC_ReadConfig(au32Config, 3) < 0) {
        printf("\nRead User Config failed!\n");       /* Error message */
        return -1;                     /* failed to read User Configuration */
    }

    /* Check if Data Flash is enabled and is expected address. */
    if ((!(au32Config[0] & 0x1)) && (au32Config[1] == u32DFBA))
        return 0;                      /* no need to modify User Configuration */

    FMC_ENABLE_CFG_UPDATE();           /* Enable User Configuration update. */

    au32Config[0] &= ~0x1;             /* Clear CONFIG0 bit 0 to enable Data Flash */
    au32Config[1] = u32DFBA;           /* Give Data Flash base address  */
		au32Config[2] = 0xFFFF5A5A;				 /* Unlock ALOCK */
		
		FMC_Erase(FMC_CONFIG_BASE);

    /* Update User Configuration settings. */
    if (FMC_WriteConfig(au32Config, 3) < 0)
        return -1;                     /* failed to write user configuration */

    printf("\nSet Data Flash base as 0x%x.\n", DATA_FLASH_TEST_BASE);  /* debug message */

    /* Perform chip reset to make new User Config take effect. */
    SYS->IPRST0 = SYS_IPRST0_CHIPRST_Msk;
    return 0;                          /* success */
}


void fill_data_pattern(uint32_t u32StartAddr, uint32_t u32EndAddr, uint32_t u32Pattern)
{
    uint32_t u32Addr;                  /* flash address */

    /* Fill flash range from u32StartAddr to u32EndAddr with data word u32Pattern. */
    for (u32Addr = u32StartAddr; u32Addr < u32EndAddr; u32Addr += 4) {
        FMC_Write(u32Addr, u32Pattern);          /* Program flash */
    }
}


int32_t  verify_data(uint32_t u32StartAddr, uint32_t u32EndAddr, uint32_t u32Pattern)
{
    uint32_t    u32Addr;               /* flash address */
    uint32_t    u32data;               /* flash data    */

    /* Verify if each data word from flash u32StartAddr to u32EndAddr be u32Pattern.  */
    for (u32Addr = u32StartAddr; u32Addr < u32EndAddr; u32Addr += 4) {
        u32data = FMC_Read(u32Addr);   /* Read a flash word from address u32Addr. */

        if (u32data != u32Pattern) {   /* Verify if data matched. */
            printf("\nFMC_Read data verify failed at address 0x%x, read=0x%x, expect=0x%x\n", u32Addr, u32data, u32Pattern);
            return -1;                 /* data verify failed */
        }
    }
    return 0;                          /* data verify OK */
}


int32_t  flash_test(uint32_t u32StartAddr, uint32_t u32EndAddr, uint32_t u32Pattern)
{
    uint32_t    u32Addr;               /* flash address */

    for (u32Addr = u32StartAddr; u32Addr < u32EndAddr; u32Addr += FMC_FLASH_PAGE_SIZE) {
        printf("    Flash test address: 0x%x    \r", u32Addr);       /* information message */

        FMC_Erase(u32Addr);            /* Erase page */

        /* Verify if page contents are all 0xFFFFFFFF */
        if (verify_data(u32Addr, u32Addr + FMC_FLASH_PAGE_SIZE, 0xFFFFFFFF) < 0) {
            printf("\nPage 0x%x erase verify failed!\n", u32Addr);   /* error message */
            return -1;                 /* Erase verify failed */
        }

        /* Write test pattern to fill the whole page. */
        fill_data_pattern(u32Addr, u32Addr + FMC_FLASH_PAGE_SIZE, u32Pattern);

        /* Verify if page contents are all equal to test pattern */
        if (verify_data(u32Addr, u32Addr + FMC_FLASH_PAGE_SIZE, u32Pattern) < 0) {
            printf("\nData verify failed!\n ");                      /* error message */
            return -1;                 /* Program verify failed */
        }

        /* Erase page */
        FMC_Erase(u32Addr);

        /* Verify if page contents are all 0xFFFFFFFF after erased. */
        if (verify_data(u32Addr, u32Addr + FMC_FLASH_PAGE_SIZE, 0xFFFFFFFF) < 0) {
            printf("\nPage 0x%x erase verify failed!\n", u32Addr);   /* error message */
            return -1;                 /* Erase verify failed */
        }
    }
    printf("\r    Flash Test Passed.          \n");                  /* information message */
    return 0;      /* flash test passed */
}


int main()
{
	uint32_t    i, u32Data;            /* variables */

	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	UART0_Init();                      /* Initialize UART0 */

	/*---------------------------------------------------------------------------------------------------------*/
	/* SAMPLE CODE                                                                                             */
	/*---------------------------------------------------------------------------------------------------------*/

	printf("+------------------------------------------+\n");
	printf("|       FMC Read/Write Sample Demo         |\n");
	printf("+------------------------------------------+\n");

	SYS_UnlockReg();                   /* Unclok register lock protect */
	
	FMC_Open();                        /* Enable FMC ISP function */
	
	/* Enable Data Flash and set base address. */
	if (set_data_flash_base(DATA_FLASH_TEST_BASE) < 0) {
		printf("Failed to set Data Flash base address!\n");          /* error message */
		goto lexit;                    /* failed to configure Data Flash, aborted */
	}

	/* Get booting source (APROM/LDROM) */
	printf("  Boot Mode ............................. ");
	if (FMC_GetBootSource() == 0)
		printf("[APROM]\n");           /* debug message */
	else {
		printf("[LDROM]\n");           /* debug message */
		printf("  WARNING: The driver sample code must execute in AP mode!\n");
		goto lexit;                    /* failed to get boot source */
	}

	u32Data = FMC_ReadCID();           /* Get company ID, should be 0xDA. */
	printf("  Company ID ............................ [0x%08x]\n", u32Data);   /* information message */

	u32Data = FMC_ReadPID();           /* Get product ID. */
	printf("  Product ID ............................ [0x%08x]\n", u32Data);   /* information message */

	for (i = 0; i < 3; i++) {          /* Get 96-bits UID. */
			u32Data = FMC_ReadUID(i);
			printf("  Unique ID %d ........................... [0x%08x]\n", i, u32Data);  /* information message */
	}

	for (i = 0; i < 4; i++) {          /* Get 4 words UCID. */
			u32Data = FMC_ReadUCID(i);
			printf("  Unique Customer ID %d .................. [0x%08x]\n", i, u32Data);  /* information message */
	}

	/* Read User Configuration CONFIG0 */
	printf("  User Config 0 ......................... [0x%08x]\n", FMC_Read(FMC_CONFIG_BASE));
	/* Read User Configuration CONFIG1 */
	printf("  User Config 1 ......................... [0x%08x]\n", FMC_Read(FMC_CONFIG_BASE+4));
	
	/* Read Data Flash base address */
	u32Data = FMC_ReadDataFlashBaseAddr();
	printf("  Data Flash Base Address ............... [0x%08x]\n", u32Data);   /* information message */

	printf("\n\nLDROM test =>\n");     /* information message */

	FMC_ENABLE_LD_UPDATE();            /* Enable LDROM update. */
	/* Execute flash program/verify test on LDROM. */
	if (flash_test(FMC_LDROM_BASE, FMC_LDROM_END, TEST_PATTERN) < 0) {
		printf("\n\nLDROM test failed!\n");        /* error message */
		goto lexit;                    /* LDROM test failed. Program aborted. */
	}

	FMC_DISABLE_LD_UPDATE();           /* Disable LDROM update. */

	printf("\n\nAPROM test =>\n");     /* information message */

	FMC_ENABLE_AP_UPDATE();            /* Enable APROM update. */

	/* Execute flash program/verify test on APROM. */
	if (flash_test(APROM_TEST_BASE, DATA_FLASH_TEST_BASE, TEST_PATTERN) < 0) {
		printf("\n\nAPROM test failed!\n");        /* debug message */
		goto lexit;                                /* LDROM test failed. Program aborted. */
	}

	FMC_DISABLE_AP_UPDATE();           /* Disable APROM update. */

	printf("\n\nData Flash test =>\n");            /* information message */

	/* Execute flash program/verify test on Data Flash. */
	if (flash_test(DATA_FLASH_TEST_BASE, DATA_FLASH_TEST_END, TEST_PATTERN) < 0) {
		printf("\n\nData flash read/write test failed!\n");
		goto lexit;                    /* flash test failed */
	}

lexit:

	FMC_Close();                       /* Disable FMC ISP function */

	SYS_LockReg();                     /* Lock protected registers */

	printf("\nFMC Sample Code Completed.\n");

	while (1);
}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
