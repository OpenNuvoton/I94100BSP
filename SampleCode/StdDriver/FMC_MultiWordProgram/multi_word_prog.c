/******************************************************************************
 * @file     multi_word_prog.c
 * @version  V1.00
 * @brief    This sample run on SRAM to show FMC multi word program function.
 *
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include <stdio.h>
#include "ConfigSysClk.h"

#include "Platform.h"

#define MULTI_WORD_PROG_LEN         512          /* The maximum length is 512. */

uint32_t    page_buff[FMC_FLASH_PAGE_SIZE/4];

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

int  multi_word_program(uint32_t start_addr)
{
    uint32_t    i;

    printf("    program address 0x%x\n", start_addr);

    FMC->ISPADDR = start_addr;
    FMC->MPDAT0  = page_buff[0];
    FMC->MPDAT1  = page_buff[1];
    FMC->MPDAT2  = page_buff[2];
    FMC->MPDAT3  = page_buff[3];
    FMC->ISPCMD  = FMC_ISPCMD_PROGRAM_MUL;
    FMC->ISPTRG  = FMC_ISPTRG_ISPGO_Msk;

    for (i = 4; i < MULTI_WORD_PROG_LEN/4; ) {
        while (FMC->MPSTS & (FMC_MPSTS_D0_Msk | FMC_MPSTS_D1_Msk))
            ;

        if (!(FMC->MPSTS & FMC_MPSTS_MPBUSY_Msk)) {
            printf("    [WARNING] busy cleared after D0D1 cleared!\n");
            i += 2;
            break;
        }

        FMC->MPDAT0 = page_buff[i++];
        FMC->MPDAT1 = page_buff[i++];

        if (i == MULTI_WORD_PROG_LEN/4)
            return 0;           // done

        while (FMC->MPSTS & (FMC_MPSTS_D2_Msk | FMC_MPSTS_D3_Msk))
            ;

        if (!(FMC->MPSTS & FMC_MPSTS_MPBUSY_Msk)) {
            printf("    [WARNING] busy cleared after D2D3 cleared!\n");
            i += 2;
            break;
        }

        FMC->MPDAT2 = page_buff[i++];
        FMC->MPDAT3 = page_buff[i++];
    }

    if (i != MULTI_WORD_PROG_LEN/4) {
        printf("    [WARNING] Multi-word program interrupted at 0x%x !!\n", i);
        return -1;
    }

    while (FMC->MPSTS & FMC_MPSTS_MPBUSY_Msk) ;

    return 0;
}

int main()
{
    uint32_t  i, addr, maddr;          /* temporary variables */
	
		// Initiate system clock(Configure in ConfigSysClk.h)
		SYSCLK_INITIATE();

    UART0_Init();                      /* Initialize UART0 */

    /*---------------------------------------------------------------------------------------------------------*/
    /* SAMPLE CODE                                                                                             */
    /*---------------------------------------------------------------------------------------------------------*/

    printf("\n\n");
    printf("+-------------------------------------+\n");
    printf("|     Multi-word Program Sample       |\n");
    printf("+-------------------------------------+\n");

		/* Unlock protected registers */
		SYS_UnlockReg();

		FMC_Open();                        /* Enable FMC ISP function */

		FMC_ENABLE_AP_UPDATE();            /* Enable APROM erase/program */

    for (addr = 0; addr < 0x20000; addr += FMC_FLASH_PAGE_SIZE) {
        printf("Multiword program APROM page 0x%x =>\n", addr);

        if (FMC_Erase(addr) < 0) {
            printf("    Erase failed!!\n");
            goto err_out;
        }

        printf("    Program...");

        for (maddr = addr; maddr < addr + FMC_FLASH_PAGE_SIZE; maddr += MULTI_WORD_PROG_LEN) {
            /* Prepare test pattern */
            for (i = 0; i < MULTI_WORD_PROG_LEN; i+=4)
                page_buff[i/4] = maddr + i;

            /* execute multi-word program */
            if (multi_word_program(maddr) < 0)
                goto err_out;
        }
        printf("    [OK]\n");

        printf("    Verify...");

        for (i = 0; i < FMC_FLASH_PAGE_SIZE; i+=4)
            page_buff[i/4] = addr + i;

        for (i = 0; i < FMC_FLASH_PAGE_SIZE; i+=4) {
            if (FMC_Read(addr+i) != page_buff[i/4]) {
                printf("\n[FAILED] Data mismatch at address 0x%x, expect: 0x%x, read: 0x%x!\n", addr+i, page_buff[i/4], FMC_Read(addr+i));
                goto err_out;
            }
        }
        printf("[OK]\n");
    }

    printf("\n\nMulti-word program demo done.\n");
    while (1);

err_out:
    printf("\n\nERROR!\n");
    while (1);
}

/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
