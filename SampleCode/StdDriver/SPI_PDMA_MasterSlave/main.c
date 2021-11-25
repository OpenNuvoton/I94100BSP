/**************************************************************************//**
 * @file     main.c
 * @version  V0.1
 * $Revision: 1 $
 * $Date: 17/03/24 11:20a $
 * @brief
 *           Demonstrate SPI data transfer with PDMA.
 *           SPI0 will be configured as Master mode and SPI1 will be configured as Slave mode.
 *           Both TX PDMA function and RX PDMA function will be enabled.
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include "Platform.h"
#include "ConfigSysClk.h"

#define SPI_MASTER_TX_DMA_CH (0)
#define SPI_MASTER_RX_DMA_CH (1)
#define SPI_SLAVE_TX_DMA_CH  (2)
#define SPI_SLAVE_RX_DMA_CH  (3)

#define TEST_COUNT           (64)

/* Function prototype declaration */
void UART_Init(void);
void SPI_Init(void);
void PDMA_Init(void);
void SpiLoopTest_WithPDMA(void);

/* Global variable declaration */
uint32_t g_au32MasterToSlaveTestPattern[TEST_COUNT];
uint32_t g_au32SlaveToMasterTestPattern[TEST_COUNT];
uint32_t g_au32MasterRxBuffer[TEST_COUNT];
uint32_t g_au32SlaveRxBuffer[TEST_COUNT];

int main(void)
{
	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	UART_Init();

	/* Init SPI */
	SPI_Init();

	PDMA_Init();

	printf("\n\n");
	printf("+--------------------------------------------------------------+\n");
	printf("|                  SPI + PDMA Sample Code                      |\n");
	printf("+--------------------------------------------------------------+\n");
	printf("\n");
	printf("Configure SPI0 as a master and SPI1 as a slave.\n");
	printf("Bit length of a transaction: 32\n");
	printf("The I/O connection for SPI0/SPI1 loopback:\n");
	printf("    SPI0_SS  (PA6) <--> SPI1_SS(PD5)\n    SPI0_CLK(PA5)  <--> SPI1_CLK(PD4)\n");
	printf("    SPI0_MISO(PA4) <--> SPI1_MISO(PD3)\n    SPI0_MOSI(PA3) <--> SPI1_MOSI(PD2)\n\n");
	printf("Please connect SPI0 with SPI1, and press any key to start transmission ...");
	getchar();
	printf("\n");

	SpiLoopTest_WithPDMA();

	printf("\n\nExit SPI driver sample code.\n");

	/* Close SPI0 */
	SPI_Close(SPI0);
	/* Close SPI1 */
	SPI_Close(SPI1);
	while(1);
}

void SPI_Init(void)
{
	/* Select PCLK0 as the clock source of SPI0 */
	CLK_SetModuleClock(SPI0_MODULE, CLK_CLKSEL2_SPI0SEL_PCLK0, MODULE_NoMsk);

	/* Select PCLK1 as the clock source of SPI1 */
	CLK_SetModuleClock(SPI1_MODULE, CLK_CLKSEL2_SPI1SEL_PCLK1, MODULE_NoMsk);

	/* Enable SPI0 peripheral clock */
	CLK_EnableModuleClock(SPI0_MODULE);
	/* Enable SPI1 peripheral clock */
	CLK_EnableModuleClock(SPI1_MODULE);

	/* Set SPI0 multi-function pins */
	SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA3MFP_Msk | SYS_GPA_MFPL_PA4MFP_Msk | SYS_GPA_MFPL_PA5MFP_Msk | SYS_GPA_MFPL_PA6MFP_Msk)) | 
	(SYS_GPA_MFPL_PA5MFP_SPI0_CLK | SYS_GPA_MFPL_PA4MFP_SPI0_MISO0 | SYS_GPA_MFPL_PA6MFP_SPI0_SS0 | SYS_GPA_MFPL_PA3MFP_SPI0_MOSI0);

	/* Configure SPI1 related multi-function pins. GPD[5:2] : SPI1_CLK, SPI1_MISO, SPI1_MOSI, SPI1_SS. */
	SYS->GPD_MFPL = (SYS->GPD_MFPL & ~(SYS_GPD_MFPL_PD3MFP_Msk | SYS_GPD_MFPL_PD4MFP_Msk | SYS_GPD_MFPL_PD5MFP_Msk | SYS_GPD_MFPL_PD2MFP_Msk)) | 
	(SYS_GPD_MFPL_PD5MFP_SPI1_SS | SYS_GPD_MFPL_PD2MFP_SPI1_MOSI | SYS_GPD_MFPL_PD3MFP_SPI1_MISO | SYS_GPD_MFPL_PD4MFP_SPI1_CLK);

	/* Configure SPI0 */
	/* Reset module */
	SYS_ResetModule(SPI0_RST);
	/* Configure SPI0 as a master, SPI clock rate 2 MHz,
	 clock idle low, 32-bit transaction, drive output on falling clock edge and latch input on rising edge. */
	SPI_Open(SPI0, SPI_MASTER, SPI_MODE_0, 32, 2000000);
	/* Enable the automatic hardware slave selection function. Select the SPI0_SS pin and configure as low-active. */
	SPI_EnableAutoSS(SPI0, SPI_SS0, SPI_SS_ACTIVE_LOW);

	/* Configure SPI1 */
	/* Reset module */
	SYS_ResetModule(SPI1_RST);
	/* Configure SPI1 as a slave, clock idle low, 32-bit transaction, drive output on falling clock edge and latch input on rising edge. */
	/* Configure SPI1 as a low level active device. SPI peripheral clock rate = f_PCLK1 */
	SPI_Open(SPI1, SPI_SLAVE, SPI_MODE_0, 32, NULL);
}

void UART_Init(void)
{
	/* Select HXT as the clock source of UART0 */
	CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));

	/* Enable UART peripheral clock */
	CLK_EnableModuleClock(UART0_MODULE);

	/* Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8) */
	SYS->GPB_MFPH = (SYS->GPB_MFPH & ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk)) | (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD);

	/* Reset UART0 module */
	SYS_ResetModule(UART0_RST);

	/* Configure UART0: 115200, 8-bit word, no parity bit, 1 stop bit. */
	UART_Open(UART0, 115200);
}

void PDMA_Init(void)
{
	/* Enable PDMA clock source */
	CLK_EnableModuleClock(PDMA_MODULE);
}

void SpiLoopTest_WithPDMA(void)
{
    uint32_t u32DataCount, u32TestCycle;
    uint32_t u32RegValue, u32Abort;
    int32_t i32Err;


    printf("\nSPI0/1 Loop test with PDMA ");

    /* Source data initiation */
    for(u32DataCount = 0; u32DataCount < TEST_COUNT; u32DataCount++)
    {
        g_au32MasterToSlaveTestPattern[u32DataCount] = 0x55000000 | (u32DataCount + 1);
        g_au32SlaveToMasterTestPattern[u32DataCount] = 0xAA000000 | (u32DataCount + 1);
    }

    /* Reset PDMA module */
    SYS_ResetModule(PDMA_RST);

    /* Enable PDMA channels */
    PDMA_Open((1 << SPI_MASTER_TX_DMA_CH) | (1 << SPI_MASTER_RX_DMA_CH) | (1 << SPI_SLAVE_RX_DMA_CH) | (1 << SPI_SLAVE_TX_DMA_CH));

    /* SPI master PDMA TX channel configuration */
    /* Set transfer width (32 bits) and transfer count */
    PDMA_SetTransferCnt(SPI_MASTER_TX_DMA_CH, PDMA_WIDTH_32, TEST_COUNT);
    /* Set source/destination address and attributes */
    PDMA_SetTransferAddr(SPI_MASTER_TX_DMA_CH, (uint32_t)g_au32MasterToSlaveTestPattern, PDMA_SAR_INC, (uint32_t)&SPI0->TX, PDMA_DAR_FIX);
    /* Set request source; set basic mode. */
    PDMA_SetTransferMode(SPI_MASTER_TX_DMA_CH, PDMA_SPI0_TX, FALSE, 0);
    /* Single request type */
    PDMA_SetBurstType(SPI_MASTER_TX_DMA_CH, PDMA_REQ_SINGLE, PDMA_BURST_128);

    /* SPI master PDMA RX channel configuration */
    /* Set transfer width (32 bits) and transfer count */
    PDMA_SetTransferCnt(SPI_MASTER_RX_DMA_CH, PDMA_WIDTH_32, TEST_COUNT);
    /* Set source/destination address and attributes */
    PDMA_SetTransferAddr(SPI_MASTER_RX_DMA_CH, (uint32_t)&SPI0->RX, PDMA_SAR_FIX, (uint32_t)g_au32MasterRxBuffer, PDMA_DAR_INC);
    /* Set request source; set basic mode. */
    PDMA_SetTransferMode(SPI_MASTER_RX_DMA_CH, PDMA_SPI0_RX, FALSE, 0);
    /* Single request type */
    PDMA_SetBurstType(SPI_MASTER_RX_DMA_CH, PDMA_REQ_SINGLE, PDMA_BURST_128);

    /* SPI slave PDMA RX channel configuration */
    /* Set transfer width (32 bits) and transfer count */
    PDMA_SetTransferCnt(SPI_SLAVE_RX_DMA_CH, PDMA_WIDTH_32, TEST_COUNT);
    /* Set source/destination address and attributes */
    PDMA_SetTransferAddr(SPI_SLAVE_RX_DMA_CH, (uint32_t)&SPI1->RX, PDMA_SAR_FIX, (uint32_t)g_au32SlaveRxBuffer, PDMA_DAR_INC);
    /* Set request source; set basic mode. */
    PDMA_SetTransferMode(SPI_SLAVE_RX_DMA_CH, PDMA_SPI1_RX, FALSE, 0);
    /* Single request type */
    PDMA_SetBurstType(SPI_SLAVE_RX_DMA_CH, PDMA_REQ_SINGLE, PDMA_BURST_128);

    /* SPI slave PDMA TX channel configuration */
    /* Set transfer width (32 bits) and transfer count */
    PDMA_SetTransferCnt(SPI_SLAVE_TX_DMA_CH, PDMA_WIDTH_32, TEST_COUNT);
    /* Set source/destination address and attributes */
    PDMA_SetTransferAddr(SPI_SLAVE_TX_DMA_CH, (uint32_t)g_au32SlaveToMasterTestPattern, PDMA_SAR_INC, (uint32_t)&SPI1->TX, PDMA_DAR_FIX);
    /* Set request source; set basic mode. */
    PDMA_SetTransferMode(SPI_SLAVE_TX_DMA_CH, PDMA_SPI1_TX, FALSE, 0);
    /* Single request type */
    PDMA_SetBurstType(SPI_SLAVE_TX_DMA_CH, PDMA_REQ_SINGLE, PDMA_BURST_128);

    /* Enable SPI slave DMA function */
    SPI_TRIGGER_RX_PDMA(SPI1);
    SPI_TRIGGER_TX_PDMA(SPI1);
    /* Enable SPI master DMA function */
    SPI_TRIGGER_TX_PDMA(SPI0);
    SPI_TRIGGER_RX_PDMA(SPI0);

    i32Err = 0;
    for(u32TestCycle = 0; u32TestCycle < 10000; u32TestCycle++)
    {
        if((u32TestCycle & 0x1FF) == 0)
            putchar('.');

        while(1)
        {
            /* Get interrupt status */
            u32RegValue = PDMA_GET_INT_STATUS();
            /* Check the DMA transfer done interrupt flag */
            if(u32RegValue & PDMA_INTSTS_TDIF_Msk)
            {
                /* Check the PDMA transfer done interrupt flags */
                if((PDMA_GET_TD_STS() & ((1 << SPI_MASTER_TX_DMA_CH) | (1 << SPI_MASTER_RX_DMA_CH) | (1 << SPI_SLAVE_TX_DMA_CH) | (1 << SPI_SLAVE_RX_DMA_CH))) ==
                        ((1 << SPI_MASTER_TX_DMA_CH) | (1 << SPI_MASTER_RX_DMA_CH) | (1 << SPI_SLAVE_TX_DMA_CH) | (1 << SPI_SLAVE_RX_DMA_CH)))
                {
                    /* Clear the DMA transfer done flags */
                    PDMA_CLR_TD_FLAG((1 << SPI_MASTER_TX_DMA_CH) | (1 << SPI_MASTER_RX_DMA_CH) | (1 << SPI_SLAVE_TX_DMA_CH) | (1 << SPI_SLAVE_RX_DMA_CH));
                    /* Disable SPI master's DMA transfer function */
                    SPI_DISABLE_TX_PDMA(SPI0);
                    SPI_DISABLE_RX_PDMA(SPI0);
                    /* Check the transfer data */
                    for(u32DataCount = 0; u32DataCount < TEST_COUNT; u32DataCount++)
                    {
                        if(g_au32MasterToSlaveTestPattern[u32DataCount] != g_au32SlaveRxBuffer[u32DataCount])
                        {
                            i32Err = 1;
                            break;
                        }
                        if(g_au32SlaveToMasterTestPattern[u32DataCount] != g_au32MasterRxBuffer[u32DataCount])
                        {
                            i32Err = 1;
                            break;
                        }
                    }

                    if(u32TestCycle >= 10000)
                        break;

                    /* Source data initiation */
                    for(u32DataCount = 0; u32DataCount < TEST_COUNT; u32DataCount++)
                    {
                        g_au32MasterToSlaveTestPattern[u32DataCount]++;
                        g_au32SlaveToMasterTestPattern[u32DataCount]++;
                    }
                    /* Re-trigger */
                    /* Slave PDMA TX channel configuration */
                    /* Set transfer width (32 bits) and transfer count */
                    PDMA_SetTransferCnt(SPI_SLAVE_TX_DMA_CH, PDMA_WIDTH_32, TEST_COUNT);
                    /* Set request source; set basic mode. */
                    PDMA_SetTransferMode(SPI_SLAVE_TX_DMA_CH, PDMA_SPI1_TX, FALSE, 0);

                    /* Slave PDMA RX channel configuration */
                    /* Set transfer width (32 bits) and transfer count */
                    PDMA_SetTransferCnt(SPI_SLAVE_RX_DMA_CH, PDMA_WIDTH_32, TEST_COUNT);
                    /* Set request source; set basic mode. */
                    PDMA_SetTransferMode(SPI_SLAVE_RX_DMA_CH, PDMA_SPI1_RX, FALSE, 0);

                    /* Master PDMA TX channel configuration */
                    /* Set transfer width (32 bits) and transfer count */
                    PDMA_SetTransferCnt(SPI_MASTER_TX_DMA_CH, PDMA_WIDTH_32, TEST_COUNT);
                    /* Set request source; set basic mode. */
                    PDMA_SetTransferMode(SPI_MASTER_TX_DMA_CH, PDMA_SPI0_TX, FALSE, 0);

                    /* Master PDMA RX channel configuration */
                    /* Set transfer width (32 bits) and transfer count */
                    PDMA_SetTransferCnt(SPI_MASTER_RX_DMA_CH, PDMA_WIDTH_32, TEST_COUNT);
                    /* Set request source; set basic mode. */
                    PDMA_SetTransferMode(SPI_MASTER_RX_DMA_CH, PDMA_SPI0_RX, FALSE, 0);

                    /* Enable SPI master DMA function */
                    SPI_TRIGGER_TX_PDMA(SPI0);
                    SPI_TRIGGER_RX_PDMA(SPI0);
                    break;
                }
            }
            /* Check the DMA transfer abort interrupt flag */
            if(u32RegValue & PDMA_INTSTS_ABTIF_Msk)
            {
                /* Get the target abort flag */
                u32Abort = PDMA_GET_ABORT_STS();
                /* Clear the target abort flag */
                PDMA_CLR_ABORT_FLAG(u32Abort);
                i32Err = 1;
                break;
            }
            /* Check the DMA time-out interrupt flag */
            if(u32RegValue & 0x000FFF00)
            {
                /* Clear the time-out flag */
                PDMA->INTSTS = u32RegValue & 0x000FFF00;
                i32Err = 1;
                break;
            }
        }

        if(i32Err)
            break;
    }

    /* Disable all PDMA channels */
    PDMA_Close();

    if(i32Err)
    {
        printf(" [FAIL]\n");
    }
    else
    {
        printf(" [PASS]\n");
    }

    return;
}


