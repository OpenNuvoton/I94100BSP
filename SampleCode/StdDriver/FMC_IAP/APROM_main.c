/******************************************************************************
 * @file     APROM_main.c
 * @version  V1.00
 * @brief    FMC APROM IAP sample program run on APROM.
 *
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>

#include "Platform.h"
#include "ConfigSysClk.h"

typedef void (FUNC_PTR)(void);

extern uint32_t  loaderImage1Base, loaderImage1Limit;   /* symbol of image start and end */

extern int IsDebugFifoEmpty(void);

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


/**
  * @brief    Check User Configuration CONFIG0 bit 6 IAP boot setting. If it's not boot with IAP
  *           mode, modify it and execute a chip reset to make it take effect.
  * @return   Is boot with IAP mode or not.
  * @retval   0   Success.
  * @retval   -1  Failed on reading or programming User Configuration.
  */
static int  set_IAP_boot_mode(void)
{
    uint32_t  au32Config[2];           /* User Configuration */

    if (FMC_ReadConfig(au32Config, 2) < 0) {     /* Read User Configuration CONFIG0 and CONFIG1. */
        printf("\nRead User Config failed!\n");
        return -1;                     /* Failed on reading User Configuration */
    }

    if (au32Config[0] & 0x40) {        /* Check if it's boot from APROM/LDROM with IAP. */
        FMC_ENABLE_CFG_UPDATE();       /* Enable User Configuration update. */
        au32Config[0] &= ~0x40;        /* Select IAP boot mode. */
        FMC_WriteConfig(au32Config, 2);/* Update User Configuration CONFIG0 and CONFIG1. */

        SYS->IPRST0 = SYS_IPRST0_CHIPRST_Msk;    /* Perform chip reset to make new User Config take effect. */
    }
    return 0;                          /* success */
}

/**
  * @brief    Load an image to specified flash address. The flash area must have been enabled by
  *           caller. For exmaple, if caller want to program an image to LDROM, FMC_ENABLE_LD_UPDATE()
  *           must be called prior to calling this function.
  * @return   Image is successfully programmed or not.
  * @retval   0   Success.
  * @retval   -1  Program/verify failed.
  */
static int  load_image_to_flash(uint32_t image_base, uint32_t image_limit, uint32_t flash_addr, uint32_t max_size)
{
    uint32_t   i, j, u32Data, u32ImageSize, *pu32Loader;

    u32ImageSize = max_size;           /* Give the maximum size of programmable flash area. */

    printf("Program image to flash address 0x%x...", flash_addr);    /* information message */

    /*
     * program the whole image to specified flash area
     */
    pu32Loader = (uint32_t *)image_base;
    for (i = 0; i < u32ImageSize; i += FMC_FLASH_PAGE_SIZE)  {

        FMC_Erase(flash_addr + i);     /* erase a flash page */
        for (j = 0; j < FMC_FLASH_PAGE_SIZE; j += 4) {               /* program image to this flash page */
            FMC_Write(flash_addr + i + j, pu32Loader[(i + j) / 4]);
        }
    }
    printf("OK.\nVerify ...");

    /* Verify loader */
    for (i = 0; i < u32ImageSize; i += FMC_FLASH_PAGE_SIZE) {
        for (j = 0; j < FMC_FLASH_PAGE_SIZE; j += 4) {
            u32Data = FMC_Read(flash_addr + i + j);        /* read a word from flash memory */

            if (u32Data != pu32Loader[(i+j)/4]) {          /* check if the word read from flash be matched with original image */
                printf("data mismatch on 0x%x, [0x%x], [0x%x]\n", flash_addr + i + j, u32Data, pu32Loader[(i+j)/4]);
                return -1;             /* image program failed */
            }

            if (i + j >= u32ImageSize) /* check if it reach the end of image */
                break;
        }
    }
    printf("OK.\n");
    return 0;                          /* success */
}


int main()
{
	uint8_t     u8Item;                /* menu item */
	uint32_t    u32Data;               /* temporary data word */
	FUNC_PTR    *func;                 /* function pointer */

	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	UART0_Init();                      /* Initialize UART0 */

	/*---------------------------------------------------------------------------------------------------------*/
	/* SAMPLE CODE                                                                                             */
	/*---------------------------------------------------------------------------------------------------------*/

	printf("\n\n");
	printf("+----------------------------------------+\n");
	printf("|     	  FMC_IAP Sample Code            |\n");
	printf("|           [APROM code]                 |\n");
	printf("+----------------------------------------+\n");

	/* Unlock protected registers */
	SYS_UnlockReg();

	FMC_Open();                        /* Enable FMC ISP function */

	/*
	 *  Check if User Configuration CBS is boot with IAP mode.
	 *  If not, modify it.
	 */
	if (set_IAP_boot_mode() < 0) {
			printf("Failed to set IAP boot mode!\n");
			goto lexit;                    /* Failed to set IAP boot mode. Program aborted. */
	}

	/* Read BS */
	printf("  Boot Mode ............................. ");
	if (FMC_GetBootSource() == 0)      /* Get boot source */
		printf("[APROM]\n");           /* Is booting from APROM */
	else 
	{
		printf("[LDROM]\n");           /* Is booting from LDROM */
		printf("  WARNING: The sample code must execute in APROM!\n");
		goto lexit;                    /* This sampe program must execute in APROM. Program aborted. */
	}

	u32Data = FMC_ReadCID();           /* get company ID */
	printf("  Company ID ............................ [0x%08x]\n", u32Data);

	u32Data = FMC_ReadPID();           /* get product ID */
	printf("  Product ID ............................ [0x%08x]\n", u32Data);

	/* Read User Configuration CONFIG0 */
	printf("  User Config 0 ......................... [0x%08x]\n", FMC_Read(FMC_CONFIG_BASE));
	/* Read User Configuration CONFIG1 */
	printf("  User Config 1 ......................... [0x%08x]\n", FMC_Read(FMC_CONFIG_BASE+4));

	do 
	{
		printf("\n\n\n");
		printf("+----------------------------------------+\n");
		printf("|               Select                   |\n");
		printf("+----------------------------------------+\n");
		printf("| [0] Load IAP code to LDROM             |\n");
		printf("| [1] Run IAP program (in LDROM)         |\n");
		printf("+----------------------------------------+\n");
		printf("Please select...");
		u8Item = getchar();            /* block waiting to receive any one character from UART0 */
		printf("%c\n", u8Item);        /* print out the selected item */

		switch (u8Item)
		{
			case '0':
				FMC_ENABLE_LD_UPDATE();    /* Enable LDROM update capability */
				/*
				 *  The binary image of LDROM code is embedded in this sample.
				 *  load_image_to_flash() will program this LDROM code to LDROM.
				 */
				if (load_image_to_flash((uint32_t)&loaderImage1Base, (uint32_t)&loaderImage1Limit,
																FMC_LDROM_BASE, FMC_LDROM_SIZE) != 0) {
						printf("Load image to LDROM failed!\n");
						goto lexit;            /* Load LDROM code failed. Program aborted. */
				}
				FMC_DISABLE_LD_UPDATE();   /* Disable LDROM update capability */
				break;

			case '1':
				printf("\n\nChange VECMAP and branch to LDROM...\n");
				while(IsDebugFifoEmpty() == 0);      /* Wait for UART3 TX FIFO cleared */

				/*  NOTE!
				 *     Before change VECMAP, user MUST disable all interrupts.
				 *     The following code CANNOT locate in address 0x0 ~ 0x200.
				 */
				NVIC->ICER[0] |= 0xFFFFFFFF;
				NVIC->ICER[1] |= 0xFFFFFFFF;
				NVIC->ICER[2] |= 0xFFFFFFFF;
				FMC_SetVectorPageAddr(FMC_LDROM_BASE);
				SYS_LockReg();                                /* Lock protected registers */
				/*
				 *  The reset handler address of an executable image is located at offset 0x4.
				 *  Thus, this sample get reset handler address of LDROM code from FMC_LDROM_BASE + 0x4.
				 */
				func = (FUNC_PTR *)*(uint32_t *)(FMC_LDROM_BASE + 4);
				/*
				 *  The stack base address of an executable image is located at offset 0x0.
				 *  Thus, this sample get stack base address of LDROM code from FMC_LDROM_BASE + 0x0.
				 */
				__set_MSP(*(uint32_t *)FMC_LDROM_BASE);
				/*
				 *  Brach to the LDROM code's reset handler in way of function call.
				 */
				func();
				break;
		default :
				continue;                  /* invalid selection */
		}
	} while (1);


lexit:                                 /* program exit */

	FMC_Close();                       /* Disable FMC ISP function */

	SYS_LockReg();                     /* Lock protected registers */

	printf("\nFMC Sample Code Completed.\n");

	while (1);
}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
