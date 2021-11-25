/****************************************************************
 *                                                              *
 * Copyright (c) Nuvoton Technology Corp. All rights reserved. *
 *                                                              *
 ****************************************************************/
#include "SPIFlash.h"
#include <string.h>


#if defined ( __CC_ARM )
#pragma O0								// This pragma changes the optimization level, level Om range: O0~O3.
#elif defined ( __ICCARM__ )
#pragma optimize=medium					// optimization level: none, low, medium and high.
#elif defined ( __GNUC__ )
#pragma GCC optimization_level 2		// optimization level range: 0~3.
#endif

// ------------------------------------------------------------------------------
// Define SPI Flash Bad Block Table Variable(Just for Nand-Flash)
// ------------------------------------------------------------------------------
#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
#define SPIFLASH_BBT_BADBLOCK 	(0xFFFE)
#define SPIFLASH_BBT_NVFORMAT 	(0x4E56)
#endif

typedef struct
{
	UINT16  u16BadBlockMarker;
	UINT16  u16UserDataII;
	UINT16  au16UserDataI[2];
	UINT8   au8ECCForSec[6];
	UINT8   au8ECCForSpare[2];
} S_SPIFLASH_NANDSPARE;

// Following API is used to reduce code size
void SPIFlash_SetWidthBurst(SPI_T *psSPIHandler, UINT32 u32Width, UINT32 u32Num)
{
	SPI_SET_DATA_WIDTH(psSPIHandler, u32Width);
// Reset After Setting Data Width
#if (defined(__NPCA121_SERIES__) || defined(__I94100_SERIES__))
		SPI_RESET_FIFO(psSPIHandler);
		while(SPI_GetStatus(psSPIHandler, SPI_TXRX_RESET_MASK) == SPI_TXRX_RESET_MASK);
#endif
	SPI_SET_TX_NUM(psSPIHandler, u32Num);
}

void SPIFlash_SetTxTrig(SPI_T *psSPIHandler, UINT32 u32Tx)
{
	SPI_WRITE_TX0(psSPIHandler, u32Tx);
	SPI_GO(psSPIHandler);
}

void SPIFlash_SetSSTxTrig(S_SPIFLASH_HANDLER *psSPIFlashHandler, UINT32 u32Tx)
{
	SPI_SET_SS(psSPIFlashHandler->psSpiHandler, psSPIFlashHandler->u8SlaveDevice);
	SPI_WRITE_TX0(psSPIFlashHandler->psSpiHandler, u32Tx);
	SPI_GO(psSPIFlashHandler->psSpiHandler);
}
//-------------------------------------------

void
SPIFlash_Dummy_Cmd(SPI_T *psSPIHandler, UINT8 u8ByteNo)
{
	UINT32 u32i;
	// Set transmit Bit Length = 8
	// Transmit/Receive Burst Number: 1
	SPIFlash_SetWidthBurst(psSPIHandler, 8, SPI_TXNUM_ONE);

	// Send Dummy data.
	for(u32i=0;u32i<u8ByteNo;u32i++)
	{
		// Trigger SPI for send data.
		SPIFlash_SetTxTrig(psSPIHandler, SPIFLASH_DUMMY);
		// Wait Tx finish.
		#if !defined(__NPCA121_SERIES__) && !defined(__I94100_SERIES__)
		while( SPI_IS_BUSY(psSPIHandler) );
		#else
		while(SPI_GET_TX_FIFO_EMPTY_FLAG(psSPIHandler) == 0);
		while(SPI_GET_RX_FIFO_EMPTY_FLAG(psSPIHandler));
		#endif
	}
}

void
SPIFlash_2ByteAddr_Cmd(SPI_T *psSPIHandler, UINT32 u32Cmd, UINT32 u32ByteAddr, UINT8 u8SlaveDevice)
{
	// Transmit/Receive Burst Number: 1
	// Set transmit Bit Length = 24
	SPIFlash_SetWidthBurst(psSPIHandler, 24, SPI_TXNUM_ONE);
	// Active chip select
	SPI_SET_SS(psSPIHandler, u8SlaveDevice);
	// Trigger SPI for send data#
	SPIFlash_SetTxTrig(psSPIHandler, ((UINT32)u32Cmd<<16)|(u32ByteAddr&0xFFFF));
	
	// Wait Tx finish.
	#if !defined(__NPCA121_SERIES__) && !defined(__I94100_SERIES__)
	while( SPI_IS_BUSY(psSPIHandler) );
	#else
	while(SPI_GET_TX_FIFO_EMPTY_FLAG(psSPIHandler) == 0);
	while(SPI_GET_RX_FIFO_EMPTY_FLAG(psSPIHandler));
	#endif
}

void
SPIFlash_3ByteAddr_Cmd(SPI_T	*psSpiHandler, UINT32 u32Cmd, UINT32 u32ByteAddr)
{
	SPI_SET_DATA_WIDTH(psSpiHandler,32);
// Reset After Setting Data Width
#if (defined(__NPCA121_SERIES__) || defined(__I94100_SERIES__))
		SPI_RESET_FIFO(psSpiHandler);
		while(SPI_GetStatus(psSpiHandler, SPI_TXRX_RESET_MASK) == SPI_TXRX_RESET_MASK);
#endif
	SPI_SET_TX_NUM(psSpiHandler,SPI_TXNUM_ONE);
	SPI_WRITE_TX0(psSpiHandler,((UINT32)u32Cmd<<24)|(u32ByteAddr&0xFFFFFF));
	SPI_GO(psSpiHandler);
#if !defined(__NPCA121_SERIES__) && !defined(__I94100_SERIES__)
  while( SPI_IS_BUSY(psSpiHandler) );
#else
  while(SPI_GET_TX_FIFO_EMPTY_FLAG(psSpiHandler) == 0);
  while(SPI_GET_RX_FIFO_EMPTY_FLAG(psSpiHandler));
#endif
}

void
SPIFlash_4ByteAddr_Cmd(SPI_T	*psSpiHandler, UINT32 u32Cmd, UINT32 u32ByteAddr)
{
	SPI_SET_DATA_WIDTH(psSpiHandler,20);
// Reset After Setting Data Width
#if (defined(__NPCA121_SERIES__) || defined(__I94100_SERIES__))
	SPI_RESET_FIFO(psSpiHandler);
	while(SPI_GetStatus(psSpiHandler, SPI_TXRX_RESET_MASK) == SPI_TXRX_RESET_MASK);
#endif
	
	SPI_SET_TX_NUM(psSpiHandler,SPI_TXNUM_TWO);
	SPI_WRITE_TX0(psSpiHandler,((UINT32)u32Cmd<<12)|((u32ByteAddr&0xfff00000)>>20));
	SPI_WRITE_TX1(psSpiHandler, (u32ByteAddr&0x000fffff));
	SPI_GO(psSpiHandler);
#if !defined(__NPCA121_SERIES__) && !defined(__I94100_SERIES__)
  while( SPI_IS_BUSY(psSpiHandler) );
#else
  while(SPI_GET_TX_FIFO_EMPTY_FLAG(psSpiHandler) == 0);
  while(SPI_GET_RX_FIFO_EMPTY_FLAG(psSpiHandler));
#endif
	SPI_SET_TX_NUM(psSpiHandler,SPI_TXNUM_ONE);
}

#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
UINT16 SPIFlash_SearchBBT(S_SPIFLASH_HANDLER *psSPIFlashHandler,UINT16 u16LogicalIdx);

void
SPIFlash_PageLoad(
	S_SPIFLASH_HANDLER *psSPIFlashHandler,
	UINT32 u32PageNum
)
{
	// Active chip select
	SPI_SET_SS(psSPIFlashHandler->psSpiHandler, psSPIFlashHandler->u8SlaveDevice);
	// Send fast read command
	SPIFlash_3ByteAddr_Cmd(psSPIFlashHandler->psSpiHandler, SPIFLASH_LOAD_PAGE, u32PageNum);
	// Inactive all slave devices
	SPI_SET_SS(psSPIFlashHandler->psSpiHandler, SPI_SS_NONE);
	// Wait erase complete
	SPIFlash_WaitReady(psSPIFlashHandler);
};

void
SPIFlash_WriteExecute(
	S_SPIFLASH_HANDLER *psSPIFlashHandler,
	UINT32 u32PageNum
)
{
	// Active chip select
	SPI_SET_SS(psSPIFlashHandler->psSpiHandler, psSPIFlashHandler->u8SlaveDevice);
	// Send fast read command
	SPIFlash_3ByteAddr_Cmd(psSPIFlashHandler->psSpiHandler, SPIFLASH_PROGRAM_EXECUTE, u32PageNum);
	// Inactive all slave devices
	SPI_SET_SS(psSPIFlashHandler->psSpiHandler, SPI_SS_NONE);
	// Wait busy
	SPIFlash_WaitReady(psSPIFlashHandler);
}

BOOL
SPIFlash_FormatBlock(
	S_SPIFLASH_HANDLER *psSPIFlashHandler,
	UINT16 u16BlockNum,
	UINT16 u16UserData
)
{
	S_SPIFLASH_NANDSPARE sNandSpareTemp;
	UINT32 u32WriteCount;
	PUINT8 pu8Data;
	UINT8 u8StatusReg;
	
	// Record Page number. 64 Pages/block.
	psSPIFlashHandler->u16PageNum = u16BlockNum*64;
		
	if( u16UserData == 0xFFFF )
	{
		// Page load.
		SPIFlash_PageLoad(psSPIFlashHandler, psSPIFlashHandler->u16PageNum);
		// Send fast read command. Read address 0x804
		SPIFlash_2ByteAddr_Cmd(psSPIFlashHandler->psSpiHandler,SPIFLASH_FAST_READ,0x804, psSPIFlashHandler->u8SlaveDevice);
		// send dummy clcok
		SPIFlash_Dummy_Cmd(psSPIFlashHandler->psSpiHandler,1);
		// read spare area data(Bad block marker, User Data I[0],[1])
		SPIFlash_ReadData(psSPIFlashHandler, (UINT8*)sNandSpareTemp.au16UserDataI, 4);
		// End read command.
		SPIFlash_ReadEnd(psSPIFlashHandler);
	}
	else
	{
		sNandSpareTemp.au16UserDataI[1] = u16UserData;
	}
	
	// Erase this block.
	SPIFlash_EraseStart(psSPIFlashHandler, SPIFLASH_128K_ERASE, (u16BlockNum<<6));
	
	// Wait erase complete
	SPIFlash_WaitReady(psSPIFlashHandler);
	// Read status to check erase success or not to decide write content into spare data.
	if( SPIFlash_ReadStatusReg( psSPIFlashHandler,eSPIFLASH_STATUS_REG3)&SPIFLASH_EFAIL )
	{
		sNandSpareTemp.u16BadBlockMarker = SPIFLASH_BBT_BADBLOCK;
		sNandSpareTemp.au16UserDataI[0] = 0;
	}
	else
	{
		sNandSpareTemp.u16BadBlockMarker = 0xFFFF;
		sNandSpareTemp.au16UserDataI[0] = SPIFLASH_BBT_NVFORMAT;
	}
	
	// Disable ECC to write data to 64-byte Spare area.
	u8StatusReg = SPIFlash_ReadStatusReg(psSPIFlashHandler,eSPIFLASH_STATUS_REG2);
	if(u8StatusReg & SPIFLASH_ECCE)
		SPIFlash_WriteStatusRegEx(psSPIFlashHandler, eSPIFLASH_STATUS_REG2, u8StatusReg&~SPIFLASH_ECCE, 24);
		
	// Page load.
	SPIFlash_PageLoad(psSPIFlashHandler, psSPIFlashHandler->u16PageNum);
	// Prepare write info.
	u32WriteCount = 8;
	pu8Data = (UINT8*)&sNandSpareTemp;
	// Write spare data
	SPIFlash_ChipWriteEnable(psSPIFlashHandler, TRUE);
	SPIFlash_2ByteAddr_Cmd(psSPIFlashHandler->psSpiHandler, SPIFLASH_RANDOM_PAGE_PROGRAM, 0x800, psSPIFlashHandler->u8SlaveDevice);
	
	// Set  Burst number:1, Data Width: 32
	SPIFlash_SetWidthBurst(psSPIFlashHandler->psSpiHandler, 32, SPI_TXNUM_ONE);

	while(u32WriteCount != 0)
	{
		// Write TX0 and Trigger SPI
		SPIFlash_SetTxTrig(psSPIFlashHandler->psSpiHandler, *((PUINT32)pu8Data));

		pu8Data += 4;
		u32WriteCount -= 4;
		while( SPI_IS_BUSY(psSPIFlashHandler->psSpiHandler) );
	}
	SPIFlash_WriteEnd(psSPIFlashHandler);
	// Write into hardware.
	SPIFlash_WriteExecute(psSPIFlashHandler, psSPIFlashHandler->u16PageNum);
	// Wait Program Execute finished.
	SPIFlash_WaitReady(psSPIFlashHandler);
	// Restore Status Register-2 value.
	if(u8StatusReg & SPIFLASH_ECCE)
		SPIFlash_WriteStatusRegEx(psSPIFlashHandler, eSPIFLASH_STATUS_REG2, u8StatusReg, 24);
	
	// return 
	return (sNandSpareTemp.u16BadBlockMarker==SPIFLASH_BBT_BADBLOCK)?FALSE:TRUE;
}

UINT16
SPIFlash_SearchBBT(
	S_SPIFLASH_HANDLER *psSPIFlashHandler,
	UINT16 u16LogicalIdx
)
{
	UINT16 u16TotalBlockCount;
	UINT8  u8i;
	// Get flash's max block count.
	u16TotalBlockCount = psSPIFlashHandler->u32FlashSize>>17;
	// Scan.
	for( u8i=0; u8i<psSPIFlashHandler->u8BBTCount; u8i++ )
	{
		if( psSPIFlashHandler->pau16BBT[u8i]==u16LogicalIdx )	
			return (u16TotalBlockCount-(u8i+1));
	}
	return u16LogicalIdx;
}

// get a new spare block to store data.
BOOL
SPIFlash_AddBBT(
	S_SPIFLASH_HANDLER *psSPIFlashHandler,
	UINT16 u16BlockIdx
)
{
	UINT16 u16TotalBlockCount, u16i;
	BOOL bIsAdded = FALSE;
	
	if( (psSPIFlashHandler->u8BBTCount==0) || (psSPIFlashHandler->pau16BBT==NULL) )
		return FALSE;
	// Get flash's total block count.
	u16TotalBlockCount=psSPIFlashHandler->u32FlashSize>>17;
	// Get the empty spare block area.
	for(u16i=0; u16i<psSPIFlashHandler->u8BBTCount; u16i++)
	{
		if( (psSPIFlashHandler->pau16BBT[u16i]==0xFFFF) && (bIsAdded == FALSE))
		{
			// Depend format success or not, write into Table.
			if( SPIFlash_FormatBlock(psSPIFlashHandler,u16TotalBlockCount-(u16i+1),u16BlockIdx) )
			{
				psSPIFlashHandler->pau16BBT[u16i] = u16BlockIdx;
				bIsAdded = TRUE;
			}
			else
				psSPIFlashHandler->pau16BBT[u16i] = SPIFLASH_BBT_BADBLOCK;
		}
		else if( psSPIFlashHandler->pau16BBT[u16i]==u16BlockIdx )
			psSPIFlashHandler->pau16BBT[u16i] = SPIFLASH_BBT_BADBLOCK;
	}
	return bIsAdded;
}

BOOL
SPIFlash_CreateBBT(
	S_SPIFLASH_HANDLER *psSPIFlashHandler,
	PUINT16 pu16BBTBuffer,
	UINT8 u8BBTCount
)
{
	UINT16 u16TotalBlockCount = 0, u16i, u16j;
	S_SPIFLASH_NANDSPARE sNandSpareTemp;

	if( psSPIFlashHandler->u32FlashSize == 0 )
		return FALSE;
	
	if( (u16TotalBlockCount=(psSPIFlashHandler->u32FlashSize>>17)) <= u8BBTCount) 
		return FALSE;
	
	memset( pu16BBTBuffer, 0xFF, u8BBTCount*sizeof(UINT16));

	// Make read mode in the buffer mode.
	SPIFlash_WriteStatusRegEx(psSPIFlashHandler,eSPIFLASH_STATUS_REG2,(SPIFlash_ReadStatusReg(psSPIFlashHandler,eSPIFLASH_STATUS_REG2)|SPIFLASH_BUF),24);	
	
	for( u16i=0; u16i<u8BBTCount; u16i++ )
	{
		// 1. Reset variable =======================================================================
		memset( &sNandSpareTemp, '\0', sizeof(S_SPIFLASH_NANDSPARE));
		
		// 2. Load first page in every block =======================================================
		SPIFlash_PageLoad(psSPIFlashHandler, ((u16TotalBlockCount-(u16i+1))*SPIFLASH_BLOCK128_SIZE)>>11);

		// 3. Load spare area data =================================================================
		// Send fast read command
		SPIFlash_2ByteAddr_Cmd(psSPIFlashHandler->psSpiHandler,SPIFLASH_FAST_READ,0x800, psSPIFlashHandler->u8SlaveDevice);
		// send dummy clcok
		SPIFlash_Dummy_Cmd(psSPIFlashHandler->psSpiHandler,1);
		// read spare area data(Bad block marker, User data II, User Data I)
		SPIFlash_ReadData(psSPIFlashHandler, (UINT8*)&sNandSpareTemp, sizeof(S_SPIFLASH_NANDSPARE));
		// End read command.
		SPIFlash_ReadEnd(psSPIFlashHandler);
		
		// 4. Check spare area data ================================================================
		if( sNandSpareTemp.u16BadBlockMarker != 0xFFFF )
			pu16BBTBuffer[u16i] = SPIFLASH_BBT_BADBLOCK;
		else
		{
			if( sNandSpareTemp.au16UserDataI[0] == SPIFLASH_BBT_NVFORMAT 			&& 
				sNandSpareTemp.au16UserDataI[1] < (u16TotalBlockCount-u8BBTCount) 	)
			{
				// Check redefine spare area to same block
				for(u16j=0; u16j<u16i; u16j++)
				{
					if(sNandSpareTemp.au16UserDataI[1]==pu16BBTBuffer[u16j])
					{
						sNandSpareTemp.au16UserDataI[1] = 0xFFFF;
						break;
					}
				}
				pu16BBTBuffer[u16i] = sNandSpareTemp.au16UserDataI[1];
			}
		}
	}
	// Set BBT info into handler to provide user search.
	psSPIFlashHandler->pau16BBT = pu16BBTBuffer;
	psSPIFlashHandler->u8BBTCount = u8BBTCount;
	return TRUE;
}

void
SPIFlash_Erase128K(
	S_SPIFLASH_HANDLER *psSPIFlashHandler,
	UINT16  u16IndexOf128K
)
{
	// Check BBT and Get the real number.
	UINT16 u16BlockIdx = SPIFlash_SearchBBT(psSPIFlashHandler,u16IndexOf128K);
	
	// if format block is fail, get a new spare block to store data.
	if( !SPIFlash_FormatBlock(psSPIFlashHandler,u16BlockIdx,0xFFFF) )
		SPIFlash_AddBBT(psSPIFlashHandler,u16IndexOf128K);
}
#else
void
SPIFlash_EN4BAddress(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
#if (SPIFLASH_OPERATION_MODE == 2)
	SPIFlash_SendRecOneData(psSpiFlashHandler,SPIFLASH_EN4B_MODE,8);
	psSpiFlashHandler->u8Flag = SPIFLASH_FLAG_HIGH_CAPACITY;
	psSpiFlashHandler->pfnSPIFlashMode = SPIFlash_4ByteAddr_Cmd;
#endif
}

void
SPIFlash_EX4BAddress(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
#if (SPIFLASH_OPERATION_MODE == 2)
	SPIFlash_SendRecOneData(psSpiFlashHandler,SPIFLASH_EX4B_MODE,8);
	psSpiFlashHandler->u8Flag = SPIFLASH_FLAG_LOW_CAPACITY;
	psSpiFlashHandler->pfnSPIFlashMode = SPIFlash_3ByteAddr_Cmd;
#endif
}
#endif

/*******************************************************************/
/*             Miscellaneous API code section                      */
/*******************************************************************/
void
SPIFlash_SendRecOneData(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32Data,
	UINT8  u8DataLen
)
{
	// Transmit/Receive Numbers = 1
	SPI_SET_TX_NUM(psSpiFlashHandler->psSpiHandler,SPI_TXNUM_ONE);

#if defined(__NPCA121_SERIES__) || defined(__I94100_SERIES__)
    SPI_RESET_FIFO(psSpiFlashHandler->psSpiHandler);
    while(SPI_GET_RX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler) == 0);
#endif

	// Active chip select
	SPI_SET_SS(psSpiFlashHandler->psSpiHandler, psSpiFlashHandler->u8SlaveDevice);
#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
	if(u8DataLen>=40)
	{
		// Set transmit Bit Length = u8DataLen
		// Write data to TX0 register
		// Trigger  for send data.
		SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler, 8);
// Reset After Setting Data Width
#if (defined(__NPCA121_SERIES__) || defined(__I94100_SERIES__))
		SPI_RESET_FIFO(psSpiFlashHandler->psSpiHandler);
		while(SPI_GetStatus(psSpiFlashHandler->psSpiHandler, SPI_TXRX_RESET_MASK) == SPI_TXRX_RESET_MASK);
#endif

		SPIFlash_SetTxTrig(psSpiFlashHandler->psSpiHandler, u32Data);

		// Wait Tx finish.
		while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
		// Set transmit Bit Length = u8DataLen
		u8DataLen -= 8;
		// Write data to TX0 register
		u32Data = 0;
	}
#endif
	
	// Set transmit Bit Length = u8DataLen
	SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler,u8DataLen);
// Reset After Setting Data Width
#if (defined(__NPCA121_SERIES__) || defined(__I94100_SERIES__))
	SPI_RESET_FIFO(psSpiFlashHandler->psSpiHandler);
	while(SPI_GetStatus(psSpiFlashHandler->psSpiHandler, SPI_TXRX_RESET_MASK) == SPI_TXRX_RESET_MASK);
#endif
	// Transmit/Receive Numbers = 1
	SPI_SET_TX_NUM(psSpiFlashHandler->psSpiHandler,SPI_TXNUM_ONE);	
	// Write data to TX0 register
	SPI_WRITE_TX0(psSpiFlashHandler->psSpiHandler,u32Data);

	SPI_GO(psSpiFlashHandler->psSpiHandler);
#if !defined(__NPCA121_SERIES__) && !defined(__I94100_SERIES__)
	while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
	// Inactive chip select
	SPI_SET_SS(psSpiFlashHandler->psSpiHandler, SPI_SS_NONE);
#else
	while(SPI_GET_TX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler) == 0);
	while(SPI_GET_RX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler));
	// Inactive chip select
	SPI_SET_SS(psSpiFlashHandler->psSpiHandler, SPI_SS_NONE);
#endif
}

BOOL
SPIFlash_CheckBusy(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
	return (SPIFlash_ReadStatusReg(psSpiFlashHandler, eSPIFLASH_STATUS_REG3)& SPIFLASH_BUSY);
#else
	return (SPIFlash_ReadStatusReg(psSpiFlashHandler, eSPIFLASH_STATUS_REG1)& SPIFLASH_BUSY);
#endif
}

void
SPIFlash_WaitReady(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
	while(SPIFlash_ReadStatusReg(psSpiFlashHandler, eSPIFLASH_STATUS_REG3)& SPIFLASH_BUSY);
#else
	while(SPIFlash_ReadStatusReg(psSpiFlashHandler, eSPIFLASH_STATUS_REG1)& SPIFLASH_BUSY);
#endif
}

UINT8
SPIFlash_ReadStatusReg(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	E_SPIFLASH_STATUS_REGISTER eStatusReg
)
{
#if ((__CHIP_SERIES__ == __ISD9300_SERIES__) || (__CHIP_SERIES__ == __I91200_SERIES__) || (__CHIP_SERIES__ == __I91200BS_SERIES__) || (__CHIP_SERIES__ == __I94100_SERIES__) || (__CHIP_SERIES__ == __NPCA121_SERIES__))
	#if(defined(SPIFLASH_SEL)&&(SPIFLASH_SEL==1))
	#else
	SPI_ClearRxFIFO(psSpiFlashHandler->psSpiHandler);
	// Wait for FIFO clear
	while (SPI_GET_RX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler) == 0);
	#endif
#endif
	
#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
	SPIFlash_SendRecOneData(psSpiFlashHandler,((SPIFLASH_READ_STATUS<<8)|eStatusReg)<<8, 24);
	return (SPI_READ_RX0(psSpiFlashHandler->psSpiHandler));
#else
	SPIFlash_SendRecOneData(psSpiFlashHandler,(SPIFLASH_READ_STATUS|eStatusReg)<<8, 16);
	return (UINT8)SPI_READ_RX0(psSpiFlashHandler->psSpiHandler);
#endif
}

void
SPIFlash_WriteStatusReg(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT8 u8Status
)
{
	SPIFlash_ChipWriteEnable(psSpiFlashHandler, TRUE);
#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
	SPIFlash_SendRecOneData(psSpiFlashHandler,(((SPIFLASH_WRITE_STATUS<<8)|eSPIFLASH_STATUS_REG1)<<8)|u8Status, 24);
#else
	SPIFlash_SendRecOneData(psSpiFlashHandler,(SPIFLASH_WRITE_STATUS<<8)|(u8Status), 16);
#endif
	SPIFlash_WaitReady(psSpiFlashHandler);
}

void
SPIFlash_WriteStatusRegEx(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	E_SPIFLASH_STATUS_REGISTER eStatusReg,
	UINT16 u16Status,
	UINT8 u8Length
)
{
#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
#else
	UINT8 shift;  // instruction occupied 8 bit, write data occupied u8Length - 8 bit
#endif

	SPIFlash_ChipWriteEnable(psSpiFlashHandler, TRUE);

#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
	SPIFlash_SendRecOneData(psSpiFlashHandler,(SPIFLASH_WRITE_STATUS<<16 | eStatusReg<<8)|u16Status, 24);
#else
	shift = u8Length - 8;
	SPIFlash_SendRecOneData(psSpiFlashHandler,((SPIFLASH_WRITE_STATUS|eStatusReg)<<shift)|u16Status, u8Length);
#endif
	
	SPIFlash_WaitReady(psSpiFlashHandler);
}

UINT32
SPIFlash_GetVersion(void)
{
	return SPIFLASH_VERSION_NUM;
}

void
SPIFlash_Open(
	SPI_T *psSpiHandler,
	UINT8 u8DrvSlaveDevice,
	UINT32 u32SpiClk,
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
	psSpiFlashHandler->u8SlaveDevice = u8DrvSlaveDevice;
	psSpiFlashHandler->psSpiHandler = psSpiHandler;
	
#if ((__CHIP_SERIES__==__N572F072__) || (__CHIP_SERIES__==__N572P072_SERIES__) || (__CHIP_SERIES__==__N571P032_SERIES__))
	if (psSpiHandler == SPI0)// Enable high speed pins
	   SYS->GPA_HS = 0x1f;
	// Configure SPI parameters
	// Mode0--> SPI RX latched rising edge of clock; TX latced falling edge of clock; SCLK idle low
	SPI_Open(psSpiFlashHandler->psSpiHandler, SPI_MASTER, SPI_MODE_0, u32SpiClk);
#elif ((__CHIP_SERIES__==__N572F065_SERIES__) || (__CHIP_SERIES__==__N572F064_SERIES__))
	// Configure SPI parameters
	// Mode0--> SPI RX latched rising edge of clock; TX latced falling edge of clock; SCLK idle low
	SPI_Open(psSpiFlashHandler->psSpiHandler, SPI_MODE_0, u32SpiClk);
#elif ((__CHIP_SERIES__==__N570F064_SERIES__)||(__CHIP_SERIES__==__N570H064_SERIES__)||(__CHIP_SERIES__==__N569S_SERIES__)||(__CHIP_SERIES__==__N575_SERIES__)\
||(__CHIP_SERIES__==__I91000_SERIES__)||(__CHIP_SERIES__==__ISD9100_SERIES__)||(__CHIP_SERIES__==__ISD9000_SERIES__))
	SPI_Open(psSpiFlashHandler->psSpiHandler, SPI_MASTER, SPI_MODE_0, u32SpiClk, 0);
#elif ((__CHIP_SERIES__ == __ISD9300_SERIES__) || (__CHIP_SERIES__ == __I91200_SERIES__) || (__CHIP_SERIES__ == __I91200BS_SERIES__))
	#if(defined(SPIFLASH_SEL)&&(SPIFLASH_SEL==1))
	SPI_Open(psSpiFlashHandler->psSpiHandler, SPI_MASTER, SPI_MODE_0, u32SpiClk, 0);
	#else
	SPI_Open(psSpiFlashHandler->psSpiHandler, SPI_MASTER, SPI_MODE_0, 8, u32SpiClk);
	#endif
#elif ((__CHIP_SERIES__ == __NPCA121_SERIES__) || (__CHIP_SERIES__ == __I94100_SERIES__))
	SPI_Open(psSpiFlashHandler->psSpiHandler, SPI_MASTER, SPI_MODE_0, 8, u32SpiClk);
#endif

	// bit MSB first
	SPI_SET_MSB_FIRST(psSpiFlashHandler->psSpiHandler);
	// send/receve command in big endian; write/read data in little endian
	SPI_DISABLE_BYTE_REORDER(psSpiFlashHandler->psSpiHandler);
	// transmit/receive word will be executed in one transfer
	SPI_SET_TX_NUM(psSpiFlashHandler->psSpiHandler, SPI_TXNUM_ONE);
	// defalut width 8 bits
	SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler, 8);
// Reset After Setting Data Width
#if (defined(__NPCA121_SERIES__) || defined(__I94100_SERIES__))
	SPI_RESET_FIFO(psSpiFlashHandler->psSpiHandler);
	while(SPI_GetStatus(psSpiFlashHandler->psSpiHandler, SPI_TXRX_RESET_MASK) == SPI_TXRX_RESET_MASK);
#endif
	// set Slave active level as low selection
	SPI_SET_SLAVE_ACTIVE_LEVEL(psSpiFlashHandler->psSpiHandler, SPI_SS_ACTIVE_LOW);
	// set Suspend Interval = 3 SCLK clock cycles for interval between two successive transmit/receive.
  // If for DUAL and QUAD transactions with REORDER, SUSPITV must be set to 0.
	SPI_SET_SUSPEND_CYCLE(psSpiFlashHandler->psSpiHandler, 0xf);
	
	psSpiFlashHandler->u32FlashSize = 0;
	psSpiFlashHandler->u8Flag = 0;
	// Inactive chip select
	SPI_SET_SS(psSpiFlashHandler->psSpiHandler, SPI_SS_NONE);

#if ((__CHIP_SERIES__ == __ISD9300_SERIES__) || (__CHIP_SERIES__ == __I91200_SERIES__) || (__CHIP_SERIES__ == __I91200BS_SERIES__))
//  we change to manually trigger after SPI_WRITE_TX0 
//	SPI_TRIGGER(psSpiFlashHandler->psSpiHandler);
#endif
	
#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
#else
	/* Set defalut 3 byte-address */ 
	SPIFlash_EX4BAddress(psSpiFlashHandler);
#endif
}

void
SPIFlash_GetChipInfo(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
	UINT8  u8CapacityOrder;
	UINT32 u32JEDECID;

 
#if ((__CHIP_SERIES__ == __ISD9300_SERIES__) || (__CHIP_SERIES__ == __I91200_SERIES__) || (__CHIP_SERIES__ == __I91200BS_SERIES__) || (__CHIP_SERIES__ == __I94100_SERIES__) || (__CHIP_SERIES__ == __NPCA121_SERIES__))
	#if(defined(SPIFLASH_SEL)&&(SPIFLASH_SEL==1))
	#else
	SPI_ClearRxFIFO(psSpiFlashHandler->psSpiHandler);
	#endif
#endif
	
	// Get JEDEC ID command to detect Winbond, MXIC and ATmel series
	// Only W25P serious not support JEDEC ID command
	u32JEDECID = SPIFlash_GetJedecID(psSpiFlashHandler);
	
	if(u32JEDECID == 0x00000000 || u32JEDECID == 0xFFFFFFFF)
	{
		psSpiFlashHandler->u32FlashSize = 0;
	}
	else
	{
		#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
		u8CapacityOrder = (((u32JEDECID)&0x0f)+(((u32JEDECID)&0xf0)>>4)*10); // based on 512Kbytes order
		if (u8CapacityOrder)
			psSpiFlashHandler->u32FlashSize = (512>>3)<<u8CapacityOrder;
		#else
		u8CapacityOrder = ((u32JEDECID)&0x0f); // based on 512Kbytes order
		if( ((u32JEDECID>>16)&0xff) == 0x1f ) // Atmel SPIFlash
		{
			u8CapacityOrder = ((u32JEDECID>>8)&0x1f); 
			u8CapacityOrder -= 1; 
		}
		if (u8CapacityOrder)
			psSpiFlashHandler->u32FlashSize = (64*1024)<<u8CapacityOrder; // Unit: 64k block bytes
			//psSpiFlashHandler->u32FlashSize = (1024*512/8)<<u16CapacityOrder;
		#endif		
	}
}

void
SPIFlash_PowerDown(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	BOOL	bEnable
)
{
	UINT8 u8Cmd;

	if ( bEnable )
		u8Cmd = SPIFLASH_POWER_DOWN;
	else
		u8Cmd = SPIFLASH_RELEASE_PD_ID;

	SPIFlash_SendRecOneData(psSpiFlashHandler,u8Cmd,8);
}

void
SPIFlash_Reset(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
	SPIFlash_SendRecOneData(psSpiFlashHandler,(UINT8)SPIFLASH_ENABLE_RESET,8);
	SPIFlash_SendRecOneData(psSpiFlashHandler,(UINT8)SPIFLASH_RESET,8);
}
/*******************************************************************/
/*             Protection API code section                         */
/*******************************************************************/
void
SPIFlash_ChipWriteEnable(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	BOOL bEnableWrite
)
{
	UINT8 u8Cmd;

	if ( bEnableWrite == TRUE )
		u8Cmd = SPIFLASH_WRITE_ENABLE;
	else
		u8Cmd = SPIFLASH_WRITE_DISABLE;
	
	SPIFlash_SendRecOneData(psSpiFlashHandler, u8Cmd, 8);
	SPIFlash_WaitReady(psSpiFlashHandler);
}

void
SPIFlash_GlobalProtect(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	BOOL bEnableGlobalProtect
)
{
	#define SPIFLASH_ALLLOCK_MASK	(0x3c)
	#define SPIFLASH_CMP_MASK		(0x4000)
	
	
#if 0 // Some new SPIFlash needs to check CMP (Complement Protect bit) value. e.g. Winbond, CMP defalut is 0

	UINT16 u16Status=0;
	
	u16Status = (SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG2)<<8)|SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG1);

#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
	SPIFlash_WriteStatusReg(psSpiFlashHandler,(bEnableGlobalProtect)?(u16Status|SPIFLASH_BP):(u16Status&~SPIFLASH_BP));
#else
	if (u16Status&SPIFLASH_CMP_MASK)
	{
		if(bEnableGlobalProtect)
			u16Status &= ~SPIFLASH_ALLLOCK_MASK;
		else
			u16Status |= SPIFLASH_ALLLOCK_MASK;
	}else
	{
		if(bEnableGlobalProtect)
			u16Status |= SPIFLASH_ALLLOCK_MASK;
		else
			u16Status &= ~SPIFLASH_ALLLOCK_MASK;
	}
		
	SPIFlash_ChipWriteEnable(psSpiFlashHandler, TRUE);

	SPIFlash_SendRecOneData(psSpiFlashHandler,((SPIFLASH_WRITE_STATUS|eSTATUS_REG1)<<8)|(u16Status&0xFF), 16);
	SPIFlash_WaitReady(psSpiFlashHandler);
	
	SPIFlash_SendRecOneData(psSpiFlashHandler,((SPIFLASH_WRITE_STATUS|eSTATUS_REG2)<<8)|((u16Status&0xFF00)>>8), 16);
	SPIFlash_WaitReady(psSpiFlashHandler);
#endif
	
#else // for old
	UINT8 u8Status=0;

	u8Status = SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG1);
	
	if(bEnableGlobalProtect)
		u8Status |= SPIFLASH_ALLLOCK_MASK;
	else
		u8Status &= ~SPIFLASH_ALLLOCK_MASK;
	
	SPIFlash_ChipWriteEnable(psSpiFlashHandler, TRUE);
	
	SPIFlash_SendRecOneData(psSpiFlashHandler,((SPIFLASH_WRITE_STATUS|eSTATUS_REG1)<<8)|(u8Status&0xFF), 16);
	SPIFlash_WaitReady(psSpiFlashHandler);
#endif
}

/*******************************************************************/
/*             Erase API code section                              */
/*******************************************************************/
void
SPIFlash_EraseStart(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT8 u8Cmd,
	UINT32 u32Addr
)
{
	SPIFlash_ChipWriteEnable(psSpiFlashHandler, TRUE);
	
	// Active chip select
	SPI_SET_SS(psSpiFlashHandler->psSpiHandler, psSpiFlashHandler->u8SlaveDevice);
	
#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
	// NAND Flash using Page address to specify erase area.
	SPIFlash_3ByteAddr_Cmd(psSpiFlashHandler->psSpiHandler, (UINT32)u8Cmd, u32Addr);
#else
	// Send erase command
#if (SPIFLASH_OPERATION_MODE == 0)
	SPIFlash_3ByteAddr_Cmd(psSpiFlashHandler->psSpiHandler, u8Cmd, u32Addr);
#elif (SPIFLASH_OPERATION_MODE == 1)
	SPIFlash_4ByteAddr_Cmd(psSpiFlashHandler->psSpiHandler, u8Cmd, u32Addr);
#else
	psSpiFlashHandler->pfnSPIFlashMode(psSpiFlashHandler->psSpiHandler, u8Cmd, u32Addr);
#endif
#endif
	
	// Inactive chip select
	SPI_SET_SS(psSpiFlashHandler->psSpiHandler, SPI_SS_NONE);
}

/*******************************************************************/
/*             Read API code section                               */
/*******************************************************************/
void
SPIFlash_Read(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32ByteAddr,
	PUINT8 pau8Data,
	UINT32 u32DataLen
)
{
#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
	UINT32 u32DataCount;

	// Process page data.
	while(u32DataLen>0)
	{
		if(u32ByteAddr&0x7FF)
		{
			u32DataCount = SPIFLASH_PAGE_SIZE-(u32ByteAddr&0x7FF);
			if(u32DataLen < u32DataCount)
				u32DataCount = u32DataLen;
		}
		else
			u32DataCount = (u32DataLen<SPIFLASH_PAGE_SIZE)?u32DataLen:SPIFLASH_PAGE_SIZE;
		
		SPIFlash_ReadStart( psSpiFlashHandler, u32ByteAddr );
		SPIFlash_ReadData(psSpiFlashHandler, pau8Data, u32DataCount);
		SPIFlash_ReadEnd(psSpiFlashHandler);
		u32DataLen -= u32DataCount;
		u32ByteAddr += u32DataCount;
		pau8Data += u32DataCount;
	}
#else
	SPIFlash_ReadStart( psSpiFlashHandler, u32ByteAddr );
	// Read data
	SPIFlash_ReadData(psSpiFlashHandler, pau8Data, u32DataLen);
	SPIFlash_ReadEnd(psSpiFlashHandler);
#endif
}

void
SPIFlash_BurstRead(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32ByteAddr,
	PUINT8 pau8Data,
	UINT32 u32DataLen
)
{
	SPIFlash_ReadStart( psSpiFlashHandler, u32ByteAddr );
	SPIFlash_ReadDataAlign(psSpiFlashHandler, pau8Data, u32DataLen);
	SPIFlash_ReadEnd(psSpiFlashHandler);
}

void
SPIFlash_ReadStart(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32ByteAddr
)
{
#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
	// Get the real block index, and record page number
	UINT32 u32BlockIdx = SPIFlash_SearchBBT(psSpiFlashHandler,u32ByteAddr>>17);
	u32ByteAddr = (u32BlockIdx<<17)+ (u32ByteAddr&(SPIFLASH_BLOCK128_SIZE-1));
	psSpiFlashHandler->u16PageNum = u32ByteAddr>>11;
	
	// Page load nand-flash
	SPIFlash_PageLoad(psSpiFlashHandler,psSpiFlashHandler->u16PageNum);
	
	// Send fast read command
	SPIFlash_2ByteAddr_Cmd(psSpiFlashHandler->psSpiHandler,SPIFLASH_FAST_READ,(u32ByteAddr&0x7FF), psSpiFlashHandler->u8SlaveDevice);
		
	// send dummy clcok
	SPIFlash_Dummy_Cmd(psSpiFlashHandler->psSpiHandler,1);
#else
	// Active chip select
	SPI_SET_SS(psSpiFlashHandler->psSpiHandler, psSpiFlashHandler->u8SlaveDevice);

	// Send Fast Read Quad Output command
#if (SPIFLASH_OPERATION_MODE == 0)
	SPIFlash_3ByteAddr_Cmd(psSpiFlashHandler->psSpiHandler, (UINT32)SPIFLASH_READ_CMD, u32ByteAddr);
#elif (SPIFLASH_OPERATION_MODE == 1)
	SPIFlash_4ByteAddr_Cmd(psSpiFlashHandler->psSpiHandler, (UINT32)SPIFLASH_READ_CMD, u32ByteAddr);
#else
	psSpiFlashHandler->pfnSPIFlashMode(psSpiFlashHandler->psSpiHandler, (UINT32)SPIFLASH_FAST_READ_4ADD, u32ByteAddr);
#endif

	// send dummy clcok
	SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler,8);
// Reset After Setting Data Width
#if (defined(__NPCA121_SERIES__) || defined(__I94100_SERIES__))
	SPI_RESET_FIFO(psSpiFlashHandler->psSpiHandler);
	while(SPI_GetStatus(psSpiFlashHandler->psSpiHandler, SPI_TXRX_RESET_MASK) == SPI_TXRX_RESET_MASK);
#endif

	SPI_WRITE_TX0(psSpiFlashHandler->psSpiHandler,0);
	//SPI_SET_TX_NUM(psSpiFlashHandler->psSpiHandler,SPI_TXNUM_ONE);
	SPI_GO(psSpiFlashHandler->psSpiHandler);
#if !defined(__NPCA121_SERIES__) && !defined(__I94100_SERIES__)
  while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
#else
//	CLK_SysTickDelay(100);
  while(SPI_GET_TX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler) == 0);
  while(SPI_GET_RX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler));
#endif
    
	// Enable read interface 
	SPIFLASH_READ_MODE(psSpiFlashHandler->psSpiHandler);
	// Set data direction as input
	SPIFLASH_INPUT_DIRECTION(psSpiFlashHandler->psSpiHandler);

#if ((__CHIP_SERIES__ == __ISD9300_SERIES__) || (__CHIP_SERIES__ == __I91200_SERIES__) || (__CHIP_SERIES__ == __I91200BS_SERIES__))
	#if(defined(SPIFLASH_SEL)&&(SPIFLASH_SEL==1))
		#else
		SPI_ClearRxFIFO(psSpiFlashHandler->psSpiHandler);
		#endif
#elif ((__CHIP_SERIES__ == __NPCA121_SERIES__) || (__CHIP_SERIES__ == __I94100_SERIES__))
  SPI_RESET_FIFO(psSpiFlashHandler->psSpiHandler);
  while(SPI_GetStatus(psSpiFlashHandler->psSpiHandler, SPI_TXRX_RESET_MASK) == SPI_TXRX_RESET_MASK);
#endif

	// send/receve command in big endian; write/read data in little endian
    // The byte REORDER function is only available in receive mode for Dual/Quad transactions
    // For DUAL and QUAD transactions with REORDER, SUSPITV must be set to 0.
	//SPI_ENABLE_BYTE_REORDER(psSpiFlashHandler->psSpiHandler);
#endif
}

void
SPIFlash_ReadEnd(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
	// Inactive all slave devices
	SPI_SET_SS(psSpiFlashHandler->psSpiHandler, SPI_SS_NONE);
	// Default mode : one-bit mode
	SPIFLASH_DEFAULT_MODE(psSpiFlashHandler->psSpiHandler);
	// send/receve command in big endian; write/read data in little endian

	//SPI_DISABLE_BYTE_REORDER(psSpiFlashHandler->psSpiHandler);

}

void
SPIFlash_ReadData(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	PUINT8 pau8Data,
	UINT32 u32DataLen
)
{
	//	PA20 CYHuang12 speedup read function.
	UINT32 u32ReadData;
	UINT8  u8ProcBytes;

	SPI_SET_TX_NUM(psSpiFlashHandler->psSpiHandler,SPI_TXNUM_ONE);
	
	u8ProcBytes = ((UINT32)pau8Data)&0x3;//u8ProcBytes = ((UINT32)pau8Data)%4;
  
	if (u8ProcBytes!=0)
	{
		u8ProcBytes = 4 - u8ProcBytes;
		if ( u8ProcBytes > u32DataLen )
			u8ProcBytes = u32DataLen;

		SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler,u8ProcBytes<<3);
// Reset After Setting Data Width
#if (defined(__NPCA121_SERIES__) || defined(__I94100_SERIES__))
	SPI_RESET_FIFO(psSpiFlashHandler->psSpiHandler);
	while(SPI_GetStatus(psSpiFlashHandler->psSpiHandler, SPI_TXRX_RESET_MASK) == SPI_TXRX_RESET_MASK);
#endif
	
#if ((__CHIP_SERIES__ == __ISD9300_SERIES__) || (__CHIP_SERIES__ == __I91200_SERIES__) || (__CHIP_SERIES__ == __I91200BS_SERIES__))
		#if(defined(SPIFLASH_SEL)&&(SPIFLASH_SEL==1))
		#else
		SPI_WRITE_TX0(psSpiFlashHandler->psSpiHandler,0);
		#endif
#elif ((__CHIP_SERIES__ == __NPCA121_SERIES__) || (__CHIP_SERIES__ == __I94100_SERIES__))
    SPI_RESET_FIFO(psSpiFlashHandler->psSpiHandler);
    while(SPI_GetStatus(psSpiFlashHandler->psSpiHandler, SPI_TXRX_RESET_MASK) == SPI_TXRX_RESET_MASK);
    SPI_WRITE_TX0(psSpiFlashHandler->psSpiHandler, 0);
#endif
		SPI_GO(psSpiFlashHandler->psSpiHandler);
		u32DataLen-=u8ProcBytes;

#if !defined(__NPCA121_SERIES__) && !defined(__I94100_SERIES__)
    while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
#else
    while(SPI_GET_TX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler) == 0);
    while(SPI_GET_RX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler));
#endif
		u32ReadData = SPI_READ_RX0(psSpiFlashHandler->psSpiHandler);

		// Not Byte Reorder.
//		*pau8Data++ = (UINT8)u32ReadData;
//		if ( u8ProcBytes >= 2 )
//		*pau8Data++ = (UINT8)(u32ReadData>>8);
//		if ( u8ProcBytes >= 3 )
//		*pau8Data++ = (UINT8)(u32ReadData>>16);
		// Byte Reorder. Do Not Use SPI_CTL.REORDER When USING Quad and Dual Mode.
		*pau8Data++ = (UINT8)(u32ReadData>>16);
		if ( u8ProcBytes >= 2 )
			*pau8Data++ = (UINT8)(u32ReadData>>8);
		if ( u8ProcBytes >= 3 )
			*pau8Data++ = (UINT8)u32ReadData;
	}

	SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler, 32);
// Reset After Setting Data Width
#if (defined(__NPCA121_SERIES__) || defined(__I94100_SERIES__))
	SPI_RESET_FIFO(psSpiFlashHandler->psSpiHandler);
	while(SPI_GetStatus(psSpiFlashHandler->psSpiHandler, SPI_TXRX_RESET_MASK) == SPI_TXRX_RESET_MASK);
#endif
	
	while (u32DataLen>=4)
	{
#if ((__CHIP_SERIES__ == __ISD9300_SERIES__) || (__CHIP_SERIES__ == __I91200_SERIES__) || (__CHIP_SERIES__ == __I91200BS_SERIES__))
		#if(defined(SPIFLASH_SEL)&&(SPIFLASH_SEL==1))
		#else
		SPI_WRITE_TX0(psSpiFlashHandler->psSpiHandler,0);
		#endif
#elif ((__CHIP_SERIES__ == __NPCA121_SERIES__) || (__CHIP_SERIES__ == __I94100_SERIES__))
		SPI_WRITE_TX0(psSpiFlashHandler->psSpiHandler,0);
#else 
		SPI_WRITE_TX0(psSpiFlashHandler->psSpiHandler,0xFFFFFFFF);
#endif
		SPI_GO(psSpiFlashHandler->psSpiHandler);
#if !defined(__NPCA121_SERIES__) && !defined(__I94100_SERIES__)
    while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
#else
    while(SPI_GET_TX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler) == 0);
    while(SPI_GET_RX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler));
#endif
		u32ReadData = SPI_READ_RX0(psSpiFlashHandler->psSpiHandler);
		
		// Byte Reorder. Do Not Use SPI_CTL.REORDER When USING Quad and Dual Mode.
		*((UINT32*)pau8Data) = __REV(u32ReadData);
		pau8Data+=4;
		u32DataLen-=4;
	}
	
	if (u32DataLen>0)
	{
		SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler, u32DataLen<<3);
// Reset After Setting Data Width
#if (defined(__NPCA121_SERIES__) || defined(__I94100_SERIES__))
	SPI_RESET_FIFO(psSpiFlashHandler->psSpiHandler);
	while(SPI_GetStatus(psSpiFlashHandler->psSpiHandler, SPI_TXRX_RESET_MASK) == SPI_TXRX_RESET_MASK);
#endif
#if ((__CHIP_SERIES__ == __ISD9300_SERIES__) || (__CHIP_SERIES__ == __I91200_SERIES__) || (__CHIP_SERIES__ == __I91200BS_SERIES__))
		#if(defined(SPIFLASH_SEL)&&(SPIFLASH_SEL==1))
		#else
		SPI_WRITE_TX0(psSpiFlashHandler->psSpiHandler,0);
		#endif
#elif ((__CHIP_SERIES__ == __NPCA121_SERIES__) || (__CHIP_SERIES__ == __I94100_SERIES__))
		SPI_WRITE_TX0(psSpiFlashHandler->psSpiHandler,0);
#endif
		SPI_GO(psSpiFlashHandler->psSpiHandler);
#if !defined(__NPCA121_SERIES__) && !defined(__I94100_SERIES__)
    while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
#else
//		CLK_SysTickDelay(100);
    while(SPI_GET_TX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler) == 0);
    while(SPI_GET_RX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler));
#endif
		u32ReadData = SPI_READ_RX0(psSpiFlashHandler->psSpiHandler);
		
		// Not Byte Reorder.
//		*pau8Data++ = (UINT8)u32ReadData;
//		if ( u32DataLen >= 2 )
//			*pau8Data++ = (UINT8)(u32ReadData>>8);
//		if ( u32DataLen >= 3 )
//			*pau8Data++ = (UINT8)(u32ReadData>>16);
		// Byte Reorder. Do Not Use SPI_CTL.REORDER When USING Quad and Dual Mode.
		if ( u32DataLen >= 3 )
			*pau8Data++ = (UINT8)(u32ReadData>>16);
		if ( u32DataLen >= 2 )
			*pau8Data++ = (UINT8)(u32ReadData>>8);

		*pau8Data++ = (UINT8)(u32ReadData);
	}
}
	
void
SPIFlash_ReadDataAlign(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	PUINT8 pau8Data,
	UINT32 u32DataLen
)
{
	UINT32 *pu32Temp = (UINT32 *)pau8Data;
	// Read data
	SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler, 32);
// Reset After Setting Data Width
#if (defined(__NPCA121_SERIES__) || defined(__I94100_SERIES__))
	SPI_RESET_FIFO(psSpiFlashHandler->psSpiHandler);
	while(SPI_GetStatus(psSpiFlashHandler->psSpiHandler, SPI_TXRX_RESET_MASK) == SPI_TXRX_RESET_MASK);
#endif
	
	SPI_SET_TX_NUM(psSpiFlashHandler->psSpiHandler,SPI_TXNUM_TWO);
	do
	{
#if ((__CHIP_SERIES__ == __ISD9300_SERIES__) || (__CHIP_SERIES__ == __I91200_SERIES__) || (__CHIP_SERIES__ == __I91200BS_SERIES__))
		#if(defined(SPIFLASH_SEL)&&(SPIFLASH_SEL==1))
		#else
		SPI_WRITE_TX0(psSpiFlashHandler->psSpiHandler,0);
		SPI_WRITE_TX1(psSpiFlashHandler->psSpiHandler,0);
		#endif
#elif ((__CHIP_SERIES__ == __NPCA121_SERIES__) || (__CHIP_SERIES__ == __I94100_SERIES__))
		SPI_RESET_FIFO(psSpiFlashHandler->psSpiHandler);
		while(SPI_GetStatus(psSpiFlashHandler->psSpiHandler, SPI_TXRX_RESET_MASK) == SPI_TXRX_RESET_MASK);
		SPI_WRITE_TX0(psSpiFlashHandler->psSpiHandler, 0);
		SPI_WRITE_TX1(psSpiFlashHandler->psSpiHandler, 0);
#else
		SPI_GO(psSpiFlashHandler->psSpiHandler);  
#endif
		
#if !defined(__NPCA121_SERIES__) && !defined(__I94100_SERIES__)
    while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
#else
    while(SPI_GET_TX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler) == 0);
    while(SPI_GET_RX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler));
#endif
		// Byte Reorder. Do Not Use SPI_CTL.REORDER When USING Quad and Dual Mode.
		*(UINT32 *)pu32Temp++ = __REV (SPI_READ_RX0(psSpiFlashHandler->psSpiHandler));
#if (defined(__NPCA121_SERIES__) || defined(__I94100_SERIES__))
		while(SPI_GET_RX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler));
#endif
		// Byte Reorder. Do Not Use SPI_CTL.REORDER When USING Quad and Dual Mode.
		*(UINT32 *)pu32Temp++ = __REV (SPI_READ_RX1(psSpiFlashHandler->psSpiHandler));
		u32DataLen -= 8;
	}while(u32DataLen>0);
}

/*******************************************************************/
/*             Write API code section                              */
/*******************************************************************/
void
SPIFlash_Write(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32Addr,
	PUINT8 pau8Data,
	UINT32 u32DataLen
)
{
	UINT32 u32WriteCount;
	
	while(u32DataLen!=0)
	{
		SPIFlash_WriteStart(psSpiFlashHandler, u32Addr);
		u32WriteCount = SPIFlash_WriteData(psSpiFlashHandler,u32Addr, pau8Data, u32DataLen);
		SPIFlash_WriteEnd(psSpiFlashHandler);
	#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
		SPIFlash_WriteExecute(psSpiFlashHandler, psSpiFlashHandler->u16PageNum);
	#endif
		// Wait write completely
		SPIFlash_WaitReady(psSpiFlashHandler);
		u32Addr += u32WriteCount;
		pau8Data += u32WriteCount;
		u32DataLen -= u32WriteCount;
	}
}

void
SPIFlash_WritePage(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32PageAddr,
	PUINT8 pau8Data
)
{

	SPIFlash_WriteStart(psSpiFlashHandler, u32PageAddr);
	SPIFlash_WriteDataAlign(psSpiFlashHandler, pau8Data);
	SPIFlash_WriteEnd(psSpiFlashHandler);

	SPIFlash_WaitReady(psSpiFlashHandler);
}

void
SPIFlash_WriteStart(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32ByteAddr
)
{
#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
	// Get the real block index.
	UINT32 u32BlockIdx = SPIFlash_SearchBBT(psSpiFlashHandler,u32ByteAddr>>17);
	u32ByteAddr = (u32BlockIdx<<17)+ (u32ByteAddr&(SPIFLASH_BLOCK128_SIZE-1));
	if( psSpiFlashHandler->u16PageNum != u32ByteAddr>>11 )
	{		
		psSpiFlashHandler->u16PageNum = u32ByteAddr>>11;
	}
	SPIFlash_PageLoad(psSpiFlashHandler,psSpiFlashHandler->u16PageNum);
	
	// Enable chip write(WEL Bit)
	SPIFlash_ChipWriteEnable(psSpiFlashHandler, TRUE);
	
	// Send wirte information.
	SPIFlash_2ByteAddr_Cmd(psSpiFlashHandler->psSpiHandler, SPIFLASH_RANDOM_PAGE_PROGRAM, (u32ByteAddr&0x7FF), psSpiFlashHandler->u8SlaveDevice);
#else
	SPIFlash_ChipWriteEnable(psSpiFlashHandler, TRUE);

	// Active chip select
	SPI_SET_SS(psSpiFlashHandler->psSpiHandler, psSpiFlashHandler->u8SlaveDevice);

		// Send Quad Input Page Program command
	#if (SPIFLASH_OPERATION_MODE == 0)
		SPIFlash_3ByteAddr_Cmd(psSpiFlashHandler->psSpiHandler, (UINT32)SPIFLASH_WRITE_CMD, u32ByteAddr);
	#elif (SPIFLASH_OPERATION_MODE == 1)
		SPIFlash_4ByteAddr_Cmd(psSpiFlashHandler->psSpiHandler, (UINT32)SPIFLASH_WRITE_CMD, u32ByteAddr);
	#else
		psSpiFlashHandler->pfnSPIFlashMode(psSpiFlashHandler->psSpiHandler, (UINT32)SPIFLASH_WRITE_CMD, u32ByteAddr);
	#endif

	// Enable write interface
	SPIFLASH_WRITE_MODE(psSpiFlashHandler->psSpiHandler);
	// Set data direction as output
	SPIFLASH_OUTPUT_DIRECTION(psSpiFlashHandler->psSpiHandler);

	// send/receve command in big endian; write/read data in little endian
	// The byte REORDER function is only available in receive mode for Dual/Quad transactions
    // For DUAL and QUAD transactions with REORDER, SUSPITV must be set to 0.
    //SPI_ENABLE_BYTE_REORDER(psSpiFlashHandler->psSpiHandler);

#endif
}

void
SPIFlash_WriteEnd(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
	// Inactive all slave devices
	SPI_SET_SS(psSpiFlashHandler->psSpiHandler, SPI_SS_NONE);
	// Disable quad mode
	SPIFLASH_DEFAULT_MODE(psSpiFlashHandler->psSpiHandler);
	// send/receve command in big endian; write/read data in little endian
	SPI_DISABLE_BYTE_REORDER(psSpiFlashHandler->psSpiHandler);
}

UINT32
SPIFlash_WriteData(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32SPIAddr,
	PUINT8 pau8Data,
	UINT32 u32DataLen
)
{
	UINT32 u32WriteCount, u32TotalWriteCount, u32ProcessByte, u32WriteData;
	
	SPI_SET_TX_NUM(psSpiFlashHandler->psSpiHandler,SPI_TXNUM_ONE);
	
#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
	u32WriteCount = SPIFLASH_PAGE_SIZE;
	if ( u32SPIAddr&0x7FF )
		 u32WriteCount -= u32SPIAddr&0x7FF;
	if ( u32WriteCount > u32DataLen )
		u32WriteCount = u32DataLen;
	u32TotalWriteCount = u32WriteCount;	
#else
	u32WriteCount = 256;
	if ( u32SPIAddr&0xff )//if ( u32SPIAddr%256 )
		 u32WriteCount -=  u32SPIAddr&0xff;//u32SPIAddr%256;
	if ( u32WriteCount > u32DataLen )
		u32WriteCount = u32DataLen;
	u32TotalWriteCount = u32WriteCount;
#endif
	if ( ((UINT32)pau8Data)&0x3 )//if ( ((UINT32)pau8Data)%4 )&0x3
	{
		u32ProcessByte = 4 - ( ((UINT32)pau8Data)&0x3 ); //((UINT32)pau8Data)%4;
		if ( u32ProcessByte > u32WriteCount )
			u32ProcessByte = u32WriteCount;

		SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler, u32ProcessByte*8);
// Reset After Setting Data Width
#if (defined(__NPCA121_SERIES__) || defined(__I94100_SERIES__))
		SPI_RESET_FIFO(psSpiFlashHandler->psSpiHandler);
		while(SPI_GetStatus(psSpiFlashHandler->psSpiHandler, SPI_TXRX_RESET_MASK) == SPI_TXRX_RESET_MASK);
#endif
		u32WriteData = *pau8Data ++;
		if ( u32ProcessByte >= 2 )
			u32WriteData |= (*pau8Data ++)<<8;
		if ( u32ProcessByte == 3 )
			u32WriteData |= (*pau8Data ++)<<16;

		// Byte Reorder. Do Not Use SPI_CTL.REORDER When USING Quad and Dual Mode.
		SPI_WRITE_TX0(psSpiFlashHandler->psSpiHandler,__REV(u32WriteData));
		SPI_GO(psSpiFlashHandler->psSpiHandler);
#if !defined(__NPCA121_SERIES__) && !defined(__I94100_SERIES__)
    while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
#else
    while(SPI_GET_TX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler) == 0);
    while(SPI_GET_RX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler));
#endif
		u32WriteCount -=  u32ProcessByte;
		//pau8Data += u32ProcessByte;
	}

	SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler, 32);
// Reset After Setting Data Width
#if (defined(__NPCA121_SERIES__) || defined(__I94100_SERIES__))
		SPI_RESET_FIFO(psSpiFlashHandler->psSpiHandler);
		while(SPI_GetStatus(psSpiFlashHandler->psSpiHandler, SPI_TXRX_RESET_MASK) == SPI_TXRX_RESET_MASK);
#endif
	SPI_SET_TX_NUM(psSpiFlashHandler->psSpiHandler, SPI_TXNUM_ONE);
	while(u32WriteCount >= 4)
	{
#if defined(__NPCA121_SERIES__) || defined(__I94100_SERIES__) 
		SPI_RESET_FIFO(psSpiFlashHandler->psSpiHandler);
		while(SPI_GetStatus(psSpiFlashHandler->psSpiHandler, SPI_TXRX_RESET_MASK) == SPI_TXRX_RESET_MASK);
#endif
		// Write data and trigger.
		SPIFlash_SetTxTrig(psSpiFlashHandler->psSpiHandler,__REV(*((PUINT32)pau8Data)));
		pau8Data += 4;
		u32WriteCount -= 4;
#if !defined(__NPCA121_SERIES__) && !defined(__I94100_SERIES__)
    while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
#else
    while(SPI_GET_TX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler) == 0);
    while(SPI_GET_RX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler));
#endif
	}
	if (u32WriteCount)
	{
		SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler, u32WriteCount * 8);
// Reset After Setting Data Width
#if (defined(__NPCA121_SERIES__) || defined(__I94100_SERIES__))
		SPI_RESET_FIFO(psSpiFlashHandler->psSpiHandler);
		while(SPI_GetStatus(psSpiFlashHandler->psSpiHandler, SPI_TXRX_RESET_MASK) == SPI_TXRX_RESET_MASK);
#endif
		
		u32WriteData = *pau8Data ++;
		if ( u32WriteCount >= 2 )
			u32WriteData |= (*pau8Data ++)<<8;
		if ( u32WriteCount == 3 )
			u32WriteData |= (*pau8Data ++)<<16;
		
		SPIFlash_SetTxTrig(psSpiFlashHandler->psSpiHandler, __REV(u32WriteData));
#if !defined(__NPCA121_SERIES__) && !defined(__I94100_SERIES__)
    while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
#else
    while(SPI_GET_TX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler) == 0);
    while(SPI_GET_RX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler));
#endif
	}
	return u32TotalWriteCount;
}

void
SPIFlash_WriteDataAlign(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	PUINT8 pau8Data
)
{
	UINT32 u32DataLen;
	UINT32 *pu32Temp = (UINT32 *)pau8Data;

	SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler, 32);
// Reset After Setting Data Width
#if (defined(__NPCA121_SERIES__) || defined(__I94100_SERIES__))
		SPI_RESET_FIFO(psSpiFlashHandler->psSpiHandler);
		while(SPI_GetStatus(psSpiFlashHandler->psSpiHandler, SPI_TXRX_RESET_MASK) == SPI_TXRX_RESET_MASK);
#endif
	
	SPI_SET_TX_NUM(psSpiFlashHandler->psSpiHandler,SPI_TXNUM_TWO);
	u32DataLen = 256;
	do
	{
#if defined(__NPCA121_SERIES__) || defined(__I94100_SERIES__)
		SPI_RESET_FIFO(psSpiFlashHandler->psSpiHandler);
		while(SPI_GetStatus(psSpiFlashHandler->psSpiHandler, SPI_TXRX_RESET_MASK) == SPI_TXRX_RESET_MASK);
#endif
		//DrvSPI_BurstWriteData(psSpiHandler,(PUINT32)pau8Data);
		// Byte Reorder. Do Not Use SPI_CTL.REORDER When USING Quad and Dual Mode.
		SPI_WRITE_TX0(psSpiFlashHandler->psSpiHandler, __REV(*pu32Temp++));
#if defined(__NPCA121_SERIES__) || defined(__I94100_SERIES__)
		while(SPI_GET_TX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler) == 0);
		while(SPI_GET_RX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler));
		SPI_RESET_FIFO(psSpiFlashHandler->psSpiHandler);
		while(SPI_GetStatus(psSpiFlashHandler->psSpiHandler, SPI_TXRX_RESET_MASK) == SPI_TXRX_RESET_MASK);
#endif
		// Byte Reorder. Do Not Use SPI_CTL.REORDER When USING Quad and Dual Mode.
		SPI_WRITE_TX1(psSpiFlashHandler->psSpiHandler, __REV(*pu32Temp++));
		SPI_GO(psSpiFlashHandler->psSpiHandler);
		//pau8Data += 8;
#if !defined(__NPCA121_SERIES__) && !defined(__I94100_SERIES__)
    while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
#else
    while(SPI_GET_TX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler) == 0);
    while(SPI_GET_RX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler));
#endif
	}while(	(u32DataLen -= 8) != 0 );
}


UINT32
SPIFlash_GetJedecID(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
	uint32_t u32Ret;
#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
	SPIFlash_SendRecOneData(psSpiFlashHandler, (UINT32)SPIFLASH_JEDEC_ID, 40);
#else
	SPIFlash_SendRecOneData(psSpiFlashHandler, (UINT32)SPIFLASH_JEDEC_ID<<24, 32);
#endif
	u32Ret = SPI_READ_RX0(psSpiFlashHandler->psSpiHandler);
	u32Ret &= 0x00ffffff;
	return u32Ret;
}


#define SPIFLASH_CHECK_READ_COUNT	100
#define SPIFLASH_CHECK_ID_COUNT		10
void SPIFlash_WaitStable(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32MaxReadVerifyCount
)
{
	UINT32 u32VerifyCount;
	
	// Make sure SPI flash digital part is ready by checking JEDEC ID
	{
		UINT32 u32CheckJedecID;
		UINT32 u32ReadJedecID;
		
		u32VerifyCount = 0;
		u32CheckJedecID = 0;
		while(u32MaxReadVerifyCount)
		{
			u32MaxReadVerifyCount --;	
			u32ReadJedecID = SPIFlash_GetJedecID(psSpiFlashHandler);
			
			if (((UINT8)u32ReadJedecID) == 0 )
			{
				// memory capacity should not be 0
				continue;
			}
			
			if (u32CheckJedecID == u32ReadJedecID)
			{
				if ( (++u32VerifyCount) == SPIFLASH_CHECK_ID_COUNT )
					break;
			}
			else
			{
				u32CheckJedecID = u32ReadJedecID;
				u32VerifyCount = 0;
			}
		}
	}
	
	// Make SPI flash leave power down mode if some where or some time had made it entring power down mode
	SPIFlash_PowerDown(psSpiFlashHandler, FALSE);

#if(!defined(SPIFLASH_NAND)||(SPIFLASH_NAND==0))
	// Check SPI flash is ready for accessing
	{
		UINT8 ui8ReadByte, ui8CheckByte;
		UINT16 u16Address;
		
		ui8CheckByte = 0;
		u32VerifyCount = 0;
		u16Address = 0;
		while(u32MaxReadVerifyCount)
		{
			u32MaxReadVerifyCount --;			
			SPIFlash_Read(psSpiFlashHandler, u16Address, &ui8ReadByte, 1);
			
			if ( (ui8ReadByte==0) || (ui8ReadByte==0xff) )
			{
				u16Address ++;
				u32VerifyCount = 0;
				continue;
			}
			
			if ( ui8ReadByte != ui8CheckByte )
			{
				ui8CheckByte = ui8ReadByte;
				u32VerifyCount = 0;
			}
			else
			{
				if ((++u32VerifyCount) == SPIFLASH_CHECK_READ_COUNT)
					break;
			}
		}
	}
#endif
}

// Fast Read Quad IO, address is sent in quad mode and can be set in continuous read mode
/*void SPIFlash_ReadQuadIO(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32StartAddr,
	PUINT8 pau8Data,
	UINT32 u32DataLen
)
{
    UINT32 u32ReadData;

	// Active chip select
	SPI_SET_SS(psSpiFlashHandler->psSpiHandler, psSpiFlashHandler->u8SlaveDevice);

	// Send fast read quad IO command in normal mode
	SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler,8);
	SPI_WRITE_TX0(psSpiFlashHandler->psSpiHandler,SPIFLASH_FAST_4READ);
	while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );

	// Enable quad mode
	SPI_ENABLE_QUAD_MODE(psSpiFlashHandler->psSpiHandler);
	// Set data direction as output to send address
	SPI_ENABLE_QUAD_OUTPUT_MODE(psSpiFlashHandler->psSpiHandler);
    
	// Send address in quad mode
	SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler,32);
	SPI_WRITE_TX0(psSpiFlashHandler->psSpiHandler,(u32StartAddr << 8));
    while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );

	// Send dummy load
	SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler,16);
	SPI_WRITE_TX0(psSpiFlashHandler->psSpiHandler, 0X0000);
    while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
    
    // Set data direction as input to receive data
    SPI_ENABLE_QUAD_INPUT_MODE(psSpiFlashHandler->psSpiHandler);
    // Set receive data width each transaction
    SPI_SET_DATA_WIDTH(psSpiFlashHandler->psSpiHandler,32);
    // Clear RX FIFO for receiving data
    SPI_ClearRxFIFO(psSpiFlashHandler->psSpiHandler);
    // Wait RX FIFO to be empty
    while (SPI_GET_RX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler) == 0);
    
    while (u32DataLen)
    {
        // Write to TX to generate SPI clock for receiving data
        SPI_WRITE_TX0(psSpiFlashHandler->psSpiHandler,0);
        while( SPI_IS_BUSY(psSpiFlashHandler->psSpiHandler) );
        // Read data from RX FIFO
        u32ReadData = SPI_READ_RX0(psSpiFlashHandler->psSpiHandler);
        *((UINT32*)pau8Data) = u32ReadData;
        // Read 4 byte each time
        pau8Data+=4;
        u32DataLen-=4;
    }
    SPI_ClearRxFIFO(psSpiFlashHandler->psSpiHandler);
    // Wait RX FIFO to be empty
    while (SPI_GET_RX_FIFO_EMPTY_FLAG(psSpiFlashHandler->psSpiHandler) == 0);

    SPI_SET_SS(psSpiFlashHandler->psSpiHandler, SPI_SS_NONE);
    SPI_DISABLE_QUAD_MODE(psSpiFlashHandler->psSpiHandler);
}*/
