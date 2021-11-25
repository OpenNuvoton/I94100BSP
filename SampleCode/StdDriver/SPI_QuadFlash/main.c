/**************************************************************************//**
 * @file     main.c
 * @version  V0.1
 * $Revision: 1 $
 * $Date: 27/03/24 11:20a $
 * @brief
 *           Demonstrate SPI data transfer.
 *           SPI0 will be configured as Master mode and SPI1 will be configured as Slave mode.
 *           SPI0(master) transfer via interrupt and SPI1(slave) transfer via main loop.
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "ConfigSysClk.h"
#include "SPIFlash.h"

//=========================================================================================
// Macro, type and constant definitions
//=========================================================================================
#define SPIFLASH_CLOCK (12000000)//(500000)
// Define the delay time before write instruction of SPI flash
#define SPIFLASH_WRITE_DELAY_TIME     (5*1000) //us
//Define the LDO stable time
#define SPIFLASH_LDO_STABLE_TIME      (0.2*1000)//us
#define SPIFLASH_STABLE_DEAY_TIME     (SPIFLASH_WRITE_DELAY_TIME+SPIFLASH_LDO_STABLE_TIME)
#define SPIFLASH_MAX_VERIFY_COUNT     ((unsigned int)((SPIFLASH_STABLE_DEAY_TIME)/(((double)((32+8)*1000000))/(SPIFLASH_CLOCK))))
#define START_BLOCK	(0)
#define END_BLOCK (START_BLOCK+1)
#define BLOCK_SIZE	64
// SPI Pin Port selection. 1: PA, 2: PC
#define PIN_SELECT	1	

//=========================================================================================
// Global Variable
//=========================================================================================
__align(4) UINT8 g_au8Buf[SPIFLASH_PAGE_SIZE];
S_SPIFLASH_HANDLER g_sSPIFlash;
UINT16 u16Status = 0;

//=========================================================================================
// Functions declaration
//=========================================================================================
void SPIFlashDemo(void);
void UART_Init(void);

int main()
{
	UINT32 u32Clock = 0;
	UINT32 u32JEDECID;

	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();
	// UART Initial.
	UART_Init();
	
	printf("\n\nCPU @ %dHz\n", SystemCoreClock);
	
#if (PIN_SELECT == 1)
	// SPI0: GPA0=MISO1, GPA1=MOSI0, GPA2=SLCK, GPA3=SSB, GPA4=MISO0, GPA5=MOSI1
	SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA1MFP_Msk | SYS_GPA_MFPL_PA2MFP_Msk | SYS_GPA_MFPL_PA3MFP_Msk | SYS_GPA_MFPL_PA4MFP_Msk | SYS_GPA_MFPL_PA5MFP_Msk | SYS_GPA_MFPL_PA6MFP_Msk)) | 
	(SYS_GPA_MFPL_PA6MFP_SPI0_SS0 | SYS_GPA_MFPL_PA5MFP_SPI0_CLK | SYS_GPA_MFPL_PA4MFP_SPI0_MISO0 | SYS_GPA_MFPL_PA3MFP_SPI0_MOSI0 | SYS_GPA_MFPL_PA2MFP_SPI0_MISO1 | SYS_GPA_MFPL_PA1MFP_SPI0_MOSI1);

	// Before setting the W25Q to Quad-Enable, MISO1 which is connected to HOLD needs to be pull high.
	// GPIO output mode to control /HOLD pin on SPIFlash.
	SYS->GPA_MFPL &= ~(SYS_GPA_MFPL_PA2MFP_Msk | SYS_GPA_MFPL_PA1MFP_Msk);
	GPIO_SetMode(PA, BIT1, GPIO_MODE_OUTPUT);
	GPIO_SetMode(PA, BIT2, GPIO_MODE_OUTPUT);
	// Set High.
	PA->DOUT |= BIT2|BIT1;
#elif (PIN_SELECT == 2)
	// Select GPA or GPC as SPI_Flash Pin.
	/* SPI0: GPC9=MISO1, GPC10=MOSI0, GPC12=SLCK, GPC7=SSB, GPC11=MISO0, GPC8=MOSI1*/
	SYS->GPC_MFPH = (SYS->GPC_MFPH & ~(SYS_GPC_MFPH_PC8MFP_Msk | SYS_GPC_MFPH_PC9MFP_Msk | SYS_GPC_MFPH_PC10MFP_Msk | SYS_GPC_MFPH_PC11MFP_Msk | SYS_GPC_MFPH_PC12MFP_Msk)) | 
	(SYS_GPC_MFPH_PC8MFP_SPI0_MOSI1 | SYS_GPC_MFPH_PC9MFP_SPI0_MISO1 | SYS_GPC_MFPH_PC10MFP_SPI0_MOSI0 | SYS_GPC_MFPH_PC11MFP_SPI0_MISO0 | SYS_GPC_MFPH_PC12MFP_SPI0_CLK);
	SYS->GPC_MFPL = (SYS->GPC_MFPL & ~(SYS_GPC_MFPL_PC7MFP_Msk))| SYS_GPC_MFPL_PC7MFP_SPI0_SS0;

	// Before setting the W25Q to Quad-Enable, MISO1 which is connected to HOLD needs to be pull high.
	// GPIO output mode to control /HOLD pin on SPIFlash.
	SYS->GPC_MFPH &= ~(SYS_GPC_MFPH_PC9MFP_SPI0_MISO1 | SYS_GPC_MFPH_PC8MFP_SPI0_MOSI1);
	GPIO_SetMode(PC, BIT9, GPIO_MODE_OUTPUT);
	GPIO_SetMode(PC, BIT8, GPIO_MODE_OUTPUT);
	PC->DOUT |= BIT8 | BIT9;
#endif

	/* Reset IP module */
	CLK_EnableModuleClock(SPI0_MODULE);
	SYS_ResetModule(SPI0_RST);
	
	SPIFlash_Open(SPI0, SPI_SS0, SPIFLASH_CLOCK, &g_sSPIFlash);
	u32Clock = SPIFlash_GetSPIClock(&g_sSPIFlash);
	printf("SPIFlash run on actual clock: %d.\n", u32Clock);
	
	printf("Press enter to start SPI flash detection....\n");
	
	if ( getchar() != 0x0d )
		while(1);
	
	// Wait SPIFlash LDO & write stable.
	SPIFlash_WaitStable(&g_sSPIFlash, SPIFLASH_MAX_VERIFY_COUNT);	
	SPIFlash_GetChipInfo(&g_sSPIFlash);
	
	if ( g_sSPIFlash.u32FlashSize == 0 )
	{
		printf("Can not find any SPI flash\n");
		while(1);
	}
	printf("\nFind a SPI flash with %d M-bit\n\n", g_sSPIFlash.u32FlashSize*8/1024/1024);

	if (g_sSPIFlash.u32FlashSize > 0x1000000) // > 128M-bits
	{
		printf("\nSPI flash is 4 byte address mode\n\n");
		SPIFlash_EN4BAddress(&g_sSPIFlash);
	}
	else
	{
		printf("\nSPI flash is 3 byte address mode\n\n");
		SPIFlash_EX4BAddress(&g_sSPIFlash);
	}
	
	SPIFlash_GlobalProtect(&g_sSPIFlash,FALSE);
	
	u32JEDECID = SPIFlash_GetJedecID(&g_sSPIFlash);
	
	printf("JEDEC ID = 0x%x\n", u32JEDECID);
	
	/* Enable QE bit in status register for quad mode*/
	SPIFlash_QuadMode_W25Q256FV(&g_sSPIFlash);
	
	// After Setting the W25Q to Quad-Enable, pins connected to HOLD and WP should set back to SPI-MFP.
#if (PIN_SELECT == 1)
	SYS->GPA_MFPL = (SYS->GPA_MFPL & ~(SYS_GPA_MFPL_PA2MFP_Msk | SYS_GPA_MFPL_PA1MFP_Msk)) | (SYS_GPA_MFPL_PA2MFP_SPI0_MISO1 | SYS_GPA_MFPL_PA1MFP_SPI0_MOSI1);
#elif (PIN_SELECT == 2)
	SYS->GPC_MFPH = (SYS->GPC_MFPH & ~(SYS_GPC_MFPH_PC8MFP_Msk | SYS_GPC_MFPH_PC9MFP_Msk)) | (SYS_GPC_MFPH_PC8MFP_SPI0_MOSI1 | SYS_GPC_MFPH_PC9MFP_SPI0_MISO1);
#endif
	
	// Check Status Register.
	u16Status = SPIFlash_ReadStatusReg(&g_sSPIFlash, eSTATUS_REG1);
	printf("Status_REG1 = 0x%x\n", u16Status);
	u16Status = SPIFlash_ReadStatusReg(&g_sSPIFlash, eSTATUS_REG2);
	printf("Status_REG2 = 0x%x\n", u16Status);
	u16Status = SPIFlash_ReadStatusReg(&g_sSPIFlash, eSTATUS_REG3);
	printf("Status_REG3 = 0x%x\n", u16Status);
	
	SPIFlashDemo();

	printf("\nSPI Flash erase/program/read sample end...\n");

	SPIFlash_Close(&g_sSPIFlash);
	CLK_DisableModuleClock(SPI0_MODULE);
	
	while(1);
}

void SPIFlashDemo(void)
{
	UINT32 u32StartAddr, u32EndAdd, /*u32StartAddr_32K, u32EndAdd_32K,*/ j;
	UINT8 u8TestPatten;

	// ------------------------------------------------------------------------------------------------------
	printf("\tStart program ...\n\n");
	u8TestPatten = 0x1;
	u32EndAdd =  BLOCK_SIZE*1024*END_BLOCK;   
	
	printf("\tErase %dth 64K block ...\n", START_BLOCK);

	SPIFlash_Erase64K(&g_sSPIFlash, START_BLOCK);
	SPIFlash_Erase4K(&g_sSPIFlash, START_BLOCK);
	
	printf("\tWrite in by Quad Input Page Program\n");
	
	for( u32StartAddr = BLOCK_SIZE*1024*START_BLOCK; u32StartAddr < u32EndAdd; u32StartAddr += SPIFLASH_PAGE_SIZE, u8TestPatten++ )
	{
		SPIFlash_Read(&g_sSPIFlash, u32StartAddr, g_au8Buf, SPIFLASH_PAGE_SIZE);
		for( j = 0; j < SPIFLASH_PAGE_SIZE; j ++ )
		{
			if ( g_au8Buf[j] != 0xff )
			{
				printf("\t\tErase block failed!\n");
				printf("\t\tAddress 0x%x, %d Elenmnet!\n", u32StartAddr, j);
				while(1);
			}
		}

		for( j = 0; j < SPIFLASH_PAGE_SIZE; j ++ )
		{
			g_au8Buf[j] = u8TestPatten+j;
		}
		
		SPIFlash_Write(&g_sSPIFlash, u32StartAddr, g_au8Buf, SPIFLASH_PAGE_SIZE);
	}

	// Read each page and check the content	 
	printf("\tStart verify pages by Fast Read Quad Output...\n");
	
	u8TestPatten = 0x01;
	
	for(u32StartAddr = BLOCK_SIZE*1024*START_BLOCK; u32StartAddr < u32EndAdd; u32StartAddr += SPIFLASH_PAGE_SIZE, u8TestPatten++ )
	{
		SPIFlash_Read(&g_sSPIFlash, u32StartAddr, g_au8Buf, SPIFLASH_PAGE_SIZE);

		for( j = 0; j < SPIFLASH_PAGE_SIZE; j ++ )
		{
			if ( g_au8Buf[j] != ((u8TestPatten + j) & 0xFF))
			{
				printf("\t\tVerify failed in Address 0x%x, %dth element!\n", u32StartAddr, j);
				printf("data is %x\n",g_au8Buf[j]);
				while(1);
			}
		}
	}
}

//=========================================================================================
// SYS
//=========================================================================================
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
