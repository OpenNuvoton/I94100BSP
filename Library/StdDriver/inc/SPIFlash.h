/****************************************************************
 *                                                              *
 * Copyright (c) Nuvoton Technology Corp. All rights reserved.  *
 *                                                              *
 ****************************************************************/

#ifndef __SPIFLASH_H__
#define __SPIFLASH_H__

// Include header file
#include "Platform.h"
//#include "SysInfra.h"

#if(!defined(SPIFLASH_NAND)) 
#warning \
SPIFLASH_NAND is undefined!\
- SPIFLASH_NAND = 1 (Turn on nand-flash function.)\
- SPIFLASH_NAND = 0 (Turn off nand-flash function. Using nor-flash)\
Please add the above definition in \
'Project'->'Options for target'->'C/C++'->'PreProcessor Symbols'->'Define'
#endif

#if ((__CHIP_SERIES__ == __ISD9300_SERIES__) || (__CHIP_SERIES__ == __I91200_SERIES__) || (__CHIP_SERIES__ == __I91200BS_SERIES__) || (__CHIP_SERIES__ == __NPCA121_SERIES__) || (__CHIP_SERIES__ == __I94100_SERIES__))
#if(!defined(SPIFLASH_INTERFACE_MODE)) 
#warning \
SPIFLASH_INTERFACE_MODE is undefined!\
- SPIFLASH_INTERFACE_MODE = 0 : One-bit Mode \
- SPIFLASH_INTERFACE_MODE = 1 : Dual Mode \
- SPIFLASH_INTERFACE_MODE = 2 : Quad Mode \
Please add the above definition in \
'Project'->'Options for target'->'C/C++'->'PreProcessor Symbols'->'Define'
#endif
#endif

#if ((__CHIP_SERIES__ == __I91200_SERIES__) || (__CHIP_SERIES__ == __I91200BS_SERIES__))
#if(!defined(SPIFLASH_SEL)) 
#warning \
SPIFLASH_SEL is undefined!\
- SPIFLASH_SEL = 1 (SPI1)\
- SPIFLASH_SEL = 0 (SPI0)\
Please add the above definition in \
'Project'->'Options for target'->'C/C++'->'PreProcessor Symbols'->'Define'
#endif
#endif

#ifdef  __cplusplus
extern "C"
{
#endif
	
// Version number
#define SPIFLASH_MAJOR_NUM	5
#define SPIFLASH_MINOR_NUM	3
#define SPIFLASH_BUILD_NUM	0
#define _SYSINFRA_VERSION(MAJOR_NUM, MINOR_NUM, BUILD_NUM)			 (((MAJOR_NUM) << 16) | ((MINOR_NUM) << 8) | (BUILD_NUM))	
#define SPIFLASH_VERSION_NUM	_SYSINFRA_VERSION(SPIFLASH_MAJOR_NUM, SPIFLASH_MINOR_NUM, SPIFLASH_BUILD_NUM)

//SPI Flash 3byte, 4byte address mode selection
#define SPIFLASH_OPERATION_MODE	(0)		//0: device is 3byte address; 1: device is 4byte address; 2: device can change 3byte/4byte address.
	
// ------------------------------------------------------------------------------
// Define the Error Code
// ------------------------------------------------------------------------------
// E_SPIFLASH_BUSY				Read/Write Data Busy
// Define an error code composed of error bit, module ID, and error ID.
#define _SYSINFRA_ERRCODE(IS_ERROR, MODULE_ID_VALUE, ERROR_ID)		((ERROR_ID) & 0xFF)
#define _MODULE_ID_SPIFLASH 163
#define E_SPIFLASH_BUSY         _SYSINFRA_ERRCODE(TRUE, _MODULE_ID_SPIFLASH, 1)

// ------------------------------------------------------------------------------
// Define SPI Flash Status1
// ------------------------------------------------------------------------------
#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
#define SPIFLASH_SRP0			0x80	// Status Register Protect Bit 0
#define SPIFLASH_BP3			0x40	// Block Protect Bit 3
#define SPIFLASH_BP2  		0x20	// Block Protect Bit 2
#define SPIFLASH_BP1			0x10	// Block Protect Bit 1
#define SPIFLASH_BP0			0x08	// Block Protect Bit 0
#define SPIFLASH_TB			0x04	// Block Protect Bit Top/Botton
#define SPIFLASH_WPE			0x02	// Write Protection Enable Bit
#define SPIFLASH_SRP1 			0x01	// Status Register Protect Bit 1
#define SPIFLASH_BP			(SPIFLASH_BP3|SPIFLASH_BP2|SPIFLASH_BP1|SPIFLASH_BP0|SPIFLASH_TB)
#else
#define SPIFLASH_SPR			0x80	// Status Register Protect
#define SPIFLASH_R				0x40	// Reserved Bit
#define SPIFLASH_BP3     		0x20	// Block Protect Bit 3
#define SPIFLASH_BP2			0x10	// Block Protect Bit 2
#define SPIFLASH_BP1			0x08	// Block Protect Bit 1
#define SPIFLASH_BP0			0x04	// Block Protect Bit 0
#define SPIFLASH_WEL			0x02	// Write Enable Latch
#define SPIFLASH_BUSY			0x01	// BUSY
#define SPIFLASH_BP				(SPIFLASH_BP3|SPIFLASH_BP2|SPIFLASH_BP1|SPIFLASH_BP0)
#endif

// ------------------------------------------------------------------------------
// Define SPI Flash Status 2
// ------------------------------------------------------------------------------
#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
#define SPIFLASH_OTPL	0x80	// OTP Data Pages Lock
#define SPIFLASH_OTPE	0x40	// Enter OTP Mode
#define SPIFLASH_SR1L 	0x20	// Status Register-1 Lock
#define SPIFLASH_ECCE	0x10	// Enable ECC
#define SPIFLASH_BUF	0x08	// Buffer Mode
#endif

// ------------------------------------------------------------------------------
// Define SPI Flash Status 3
// ------------------------------------------------------------------------------
#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
#define SPIFLASH_LUTF	 0x40	// Status Register Protect Bit 0
#define SPIFLASH_ECC    0x10	// Block Protect Bit 2
#define SPIFLASH_PFAIL  0x08	// Block Protect Bit 1
#define SPIFLASH_EFAIL  0x04	// Block Protect Bit 0
#define SPIFLASH_WEL    0x02	// Write Protection Enable Bit
#define SPIFLASH_BUSY   0x01	// Status Register Protect Bit 1
#endif

// ------------------------------------------------------------------------------
// Define SPI Flash Instruction Set
// ------------------------------------------------------------------------------
#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
#define SPIFLASH_LOAD_PAGE   			(0x13)
#define SPIFLASH_RANDOM_PAGE_PROGRAM	(0x84)
#define SPIFLASH_PROGRAM_EXECUTE       (0x10)
#define SPIFLASH_128K_ERASE            (0xD8)  
#endif

#define SPIFLASH_ZERO			0x00
#define SPIFLASH_DUMMY			0xFF

#define SPIFLASH_WRITE_ENABLE	0x06
#define SPIFLASH_WRITE_DISABLE	0x04
#define SPIFLASH_READ_STATUS	0x05
#define SPIFLASH_WRITE_STATUS	0x01

#define SPIFLASH_READ_DATA		0x03	// Using fast read to replace normal read
#define SPIFLASH_FAST_READ		0x0B
#define SPIFLASH_FAST_READ_4ADD		0x0C
#define SPIFLASH_FAST_DREAD		0x3B	// Address is one bits per clock.
#define SPIFLASH_FAST_2READ		0xBB	// Address is two bits per clock, This reduced instruction overhead
#define SPIFLASH_FAST_QREAD		0x6B	// Address is one bits per clock.
#define SPIFLASH_FAST_4READ		0xEB	// Address is four bits per clock, This reduced instruction overhead.

#define SPIFLASH_PAGE_PROGRAM	0x02
#define SPIFLASH_QPAGE_PROGRAM  0x32

#define SPIFLASH_4K_ERASE		0x20
#define SPIFLASH_32K_ERASE		0x52
#define SPIFLASH_64K_ERASE		0xD8
#define SPIFLASH_SUSPEND_EP		0x75
#define SPIFLASH_RESUME_EP		0x7A

#define SPIFLASH_CHIP_ERASE		0xC7
#define SPIFLASH_POWER_DOWN		0xB9
#define SPIFLASH_RELEASE_PD_ID	0xAB
#define SPIFLASH_DEVICE_ID		0x90
#define SPIFLASH_JEDEC_ID		0x9F
#define SPIFLASH_READ_SFDP		0x5A
#define SPIFLASH_EN4B_MODE		0xB7
#define SPIFLASH_EX4B_MODE		0xE9
#define SPIFLASH_ENABLE_RESET		0x66
#define SPIFLASH_RESET				0x99
// ------------------------------------------------------------------------------
// Define SPI Flash Page Size
// ------------------------------------------------------------------------------
#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
#define SPIFLASH_PAGE_SIZE	(2048)
#else
#define SPIFLASH_PAGE_SIZE			(256)
#endif
#define SPIFLASH_SECTOR_SIZE 		(4096)
#define SPIFLASH_BLOCK32_SIZE 	(32768)
#define SPIFLASH_BLOCK64_SIZE 	(65536)
#define SPIFLASH_BLOCK128_SIZE		(131072)

// ------------------------------------------------------------------------------
// Define SPI Flash Property Flag
// ------------------------------------------------------------------------------
#define SPIFLASH_FLAG_LOW_CAPACITY		0x00  
#define SPIFLASH_FLAG_HIGH_CAPACITY		0x01
#define SPIFLASH_FLAG_DUAL				0x02
#define SPIFLASH_FLAG_QUAD				0x04


#if ((__CHIP_SERIES__ == __ISD9300_SERIES__) || (__CHIP_SERIES__ == __I91200_SERIES__) || (__CHIP_SERIES__ == __I91200BS_SERIES__))
#if(defined(SPIFLASH_SEL)&&(SPIFLASH_SEL==1))
	#define SPI_T						SPI1_T
	#define SPI_Open					SPI1_Open
	#define SPI_Close					SPI1_Close
	#define SPI_SetBusClock				SPI1_SetBusClock
	#define SPI_SetVarClock				SPI1_SetVarClock
	#define SPI_GetBusClock				SPI1_GetBusClock
	#define SPI_GetVarClock				SPI1_GetVarClock
	#define SPI_GET_STATUS				SPI1_GET_STATUS
	#define SPI_ENABLE_FIFO				SPI1_ENABLE_FIFO
	#define SPI_DISABLE_FIFO			SPI1_DISABLE_FIFO
	#define SPI_GET_RX_FIFO_EMPTY_FLAG	SPI1_GET_RX_FIFO_EMPTY_FLAG
	#define SPI_GET_RX_FIFO_FULL_FLAG		SPI1_GET_RX_FIFO_FULL_FLAG
	#define SPI_GET_TX_FIFO_EMPTY_FLAG	SPI1_GET_TX_FIFO_EMPTY_FLAG
	#define SPI_GET_TX_FIFO_FULL_FLAG		SPI1_GET_TX_FIFO_FULL_FLAG
	#define SPI_READ_RX0				SPI1_READ_RX0
	#define SPI_READ_RX1				SPI1_READ_RX1
	#define SPI_WRITE_TX0				SPI1_WRITE_TX0
	#define SPI_WRITE_TX1				SPI1_WRITE_TX1
	#define SPI_SET_SS					SPI1_SET_SS
	#define SPI_SET_SLAVE_ACTIVE_LEVEL	SPI1_SET_SLAVE_ACTIVE_LEVEL
	#define SPI_ENABLE_BYTE_REORDER		SPI1_ENABLE_BYTE_REORDER
	#define SPI_DISABLE_BYTE_REORDER	SPI1_DISABLE_BYTE_REORDER
	#define SPI_SET_SUSPEND_CYCLE		SPI1_SET_SUSPEND_CYCLE
	#define SPI_SET_LSB_FIRST			SPI1_SET_LSB_FIRST
	#define SPI_SET_MSB_FIRST			SPI1_SET_MSB_FIRST
	#define SPI_SET_DATA_WIDTH			SPI1_SET_DATA_WIDTH
	#define SPI_SET_TX_NUM				SPI1_SET_TX_NUM
	#define SPI_IS_BUSY					SPI1_IS_BUSY
	#define SPI_GO						SPI1_GO
	#define SPI_ENABLE_AUTOSS			SPI1_ENABLE_AUTOSS
	#define SPI_DISABLE_AUTOSS			SPI1_DISABLE_AUTOSS
	#define SPI_TRIGGER_RX_PDMA			SPI1_TRIGGER_RX_PDMA
	#define SPI_TRIGGER_TX_PDMA			SPI1_TRIGGER_TX_PDMA
	#define SPI_DISABLE_RX_PDMA			SPI1_DISABLE_RX_PDMA
	#define SPI_DISABLE_TX_PDMA			SPI1_DISABLE_TX_PDMA
	#define SPI_MODE_0            SPI1_MODE_0        
	#define SPI_MODE_1            SPI1_MODE_1        
	#define SPI_MODE_2            SPI1_MODE_2        
	#define SPI_MODE_3            SPI1_MODE_3        
	#define SPI_SLAVE             SPI1_SLAVE         
	#define SPI_MASTER            SPI1_MASTER        
	#define SPI_SS_NONE           SPI1_SS_NONE       
	#define SPI_SS0               SPI1_SS0           
	#define SPI_SS1               SPI1_SS1           
	#define SPI_SS_ACTIVE_HIGH    SPI1_SS_ACTIVE_HIGH
	#define SPI_SS_ACTIVE_LOW     SPI1_SS_ACTIVE_LOW 
	#define SPI_TXNUM_ONE         SPI1_TXNUM_ONE     
	#define SPI_TXNUM_TWO         SPI1_TXNUM_TWO
#else
	#define SPI_T													SPI0_T
	#define SPI_WRITE_TX0									SPI0_WRITE_TX
	#define SPI_WRITE_TX1									SPI0_WRITE_TX
	#define SPI_READ_RX0 									SPI0_READ_RX
	#define SPI_READ_RX1 									SPI0_READ_RX
	//I92100 SPI is triggered in  SPIFlash_Open and doesn't need to trigger every time when transaction begin
	#define SPI_GO(spi)										SPI0_TRIGGER(spi)
	// Link SPI_SET_TX_NUM(spi,u32TxNum) to a dummy function due to it is not available at SPI0 in I92100
	#define SPI_SET_TX_NUM(spi,u32TxNum)	SPIFlash_Dummy(spi)
	#define SPI_TXNUM_ONE									0 //not available in I92100
	#define SPI_TXNUM_TWO									0 //not available in I92100
	#define SPI_Open											SPI0_Open
	#define SPI_Close											SPI0_Close
	#define SPI_ClearRxFIFO								SPI0_ClearRxFIFO
	#define SPI_MASTER										SPI0_MASTER
	#define SPI_SLAVE											SPI0_SLAVE
	#define SPI_MODE_0										SPI0_MODE_0
	#define SPI_MODE_1										SPI0_MODE_1
	#define SPI_MODE_2										SPI0_MODE_2
	#define SPI_MODE_3										SPI0_MODE_3
	#define SPI_GetBusClock								SPI0_GetBusClock
	#define SPI_SET_DATA_WIDTH						SPI0_SET_DATA_WIDTH
	#define SPI_IS_BUSY										SPI0_IS_BUSY
	#define SPI_SET_MSB_FIRST 						SPI0_SET_MSB_FIRST
	#define SPI_DISABLE_BYTE_REORDER 			SPI0_DISABLE_BYTE_REORDER
	#define SPI_SS_ACTIVE_LOW 						SPI0_SS_ACTIVE_LOW
	#define SPI_TRIGGER										SPI0_TRIGGER
	#define SPI_SET_SUSPEND_CYCLE 				SPI0_SET_SUSPEND_CYCLE
	#define SPI_GET_RX_FIFO_EMPTY_FLAG		SPI0_GET_RX_FIFO_EMPTY_FLAG
	#define SPI_SS_NONE										(0x0ul<<SPI0_SSCTL_SS_Pos)
	#define SPI_SET_SS(spi,u32SS)					( (spi)->SSCTL = ( (spi)->SSCTL & ~SPI0_SSCTL_SS_Msk ) | u32SS )
	#define SPI_SET_SLAVE_ACTIVE_LEVEL(spi,u32Level)	( (spi)->SSCTL = ( (spi)->SSCTL & ~SPI0_SSCTL_SSACTPOL_Msk ) | u32Level )
#endif

#if(defined(SPIFLASH_INTERFACE_MODE)&&(SPIFLASH_INTERFACE_MODE==1))	//Dual mode
#define SPIFLASH_READ_CMD					SPIFLASH_FAST_DREAD
#define SPIFLASH_WRITE_CMD					SPIFLASH_PAGE_PROGRAM
#define SPIFLASH_READ_MODE(spi)				SPI0_ENABLE_DUAL_MODE(spi)
#define SPIFLASH_WRITE_MODE(spi)			SPI0_DISABLE_DUAL_MODE(spi)
#define SPIFLASH_DEFAULT_MODE(spi)			SPI0_DISABLE_DUAL_MODE(spi)
#define SPIFLASH_OUTPUT_DIRECTION(spi)	// Write operation is standard mode, SPIFlash didn't supports Dual PROGRAM!
#define SPIFLASH_INPUT_DIRECTION(spi)		SPI0_ENABLE_DUAL_INPUT_MODE(spi)
#elif(defined(SPIFLASH_INTERFACE_MODE)&&(SPIFLASH_INTERFACE_MODE==2))	//Quad mode
#define SPIFLASH_READ_CMD					SPIFLASH_FAST_QREAD
#define SPIFLASH_WRITE_CMD					SPIFLASH_QPAGE_PROGRAM
#define SPIFLASH_READ_MODE(spi)				SPI0_ENABLE_QUAD_MODE(spi)
#define SPIFLASH_WRITE_MODE(spi)			SPI0_ENABLE_QUAD_MODE(spi)
#define SPIFLASH_DEFAULT_MODE(spi)			SPI0_DISABLE_QUAD_MODE(spi)
#define SPIFLASH_OUTPUT_DIRECTION(spi)		SPI0_ENABLE_QUAD_OUTPUT_MODE(spi)
#define SPIFLASH_INPUT_DIRECTION(spi)		SPI0_ENABLE_QUAD_INPUT_MODE(spi)
#else //one-bit mode
#define SPIFLASH_READ_CMD					SPIFLASH_FAST_READ
#define SPIFLASH_WRITE_CMD					SPIFLASH_PAGE_PROGRAM
#define SPIFLASH_READ_MODE(spi)						
#define SPIFLASH_WRITE_MODE(spi)
#define SPIFLASH_DEFAULT_MODE(spi)
#define SPIFLASH_OUTPUT_DIRECTION(spi)	
#define SPIFLASH_INPUT_DIRECTION(spi)	
#endif
#endif

#if ((__CHIP_SERIES__ == __NPCA121_SERIES__) || (__CHIP_SERIES__ == __I94100_SERIES__))

#define SPI_SS_NONE																		(0x0ul<<SPI_SSCTL_SS0_Pos)
#define SPI_SET_SS(spi,u32SS)													( (spi)->SSCTL = ( (spi)->SSCTL & ~(SPI_SSCTL_SS0_Msk|SPI_SSCTL_SS1_Msk)) | u32SS )
#define SPI_SET_SLAVE_ACTIVE_LEVEL(spi,u32Level)			( (spi)->SSCTL = ( (spi)->SSCTL & ~SPI_SSCTL_SSACTPOL_Msk ) | u32Level )
// Link SPI_SET_TX_NUM(spi,u32TxNum) to a dummy function due to it is not available at SPI0 in I92100
#define SPI_SET_TX_NUM(spi,u32TxNum)									SPIFlash_Dummy(spi)
#define SPI_TXNUM_ONE																	0 //not available
#define SPI_TXNUM_TWO																	0 //not available
#define SPI_WRITE_TX0																	SPI_WRITE_TX
#define SPI_WRITE_TX1																	SPI_WRITE_TX
#define SPI_READ_RX0																	SPI_READ_RX
#define SPI_READ_RX1																	SPI_READ_RX
#define SPI_GO(spi)																		//SPI is triggered in  SPIFlash_Open and doesn't need to trigger every time when transaction begin



#if(defined(SPIFLASH_INTERFACE_MODE)&&(SPIFLASH_INTERFACE_MODE==1))	//Dual mode
#define SPIFLASH_READ_CMD					SPIFLASH_FAST_DREAD
#define SPIFLASH_WRITE_CMD					SPIFLASH_PAGE_PROGRAM
#define SPIFLASH_READ_MODE(spi)				SPI_ENABLE_DUAL_INPUT_MODE(spi)
#define SPIFLASH_WRITE_MODE(spi)			SPI_DISABLE_DUAL_MODE(spi)
#define SPIFLASH_DEFAULT_MODE(spi)			SPI_DISABLE_DUAL_MODE(spi)
#define SPIFLASH_OUTPUT_DIRECTION(spi)	// Write operation is standard mode, SPIFlash didn't supports Dual PROGRAM!
#define SPIFLASH_INPUT_DIRECTION(spi)		SPI_ENABLE_DUAL_INPUT_MODE(spi)
#elif(defined(SPIFLASH_INTERFACE_MODE)&&(SPIFLASH_INTERFACE_MODE==2))	//Quad mode
#define SPIFLASH_READ_CMD					SPIFLASH_FAST_QREAD
#define SPIFLASH_WRITE_CMD					SPIFLASH_QPAGE_PROGRAM
#define SPIFLASH_READ_MODE(spi)				SPI_ENABLE_QUAD_INPUT_MODE(spi)
#define SPIFLASH_WRITE_MODE(spi)			SPI_ENABLE_QUAD_OUTPUT_MODE(spi)
#define SPIFLASH_DEFAULT_MODE(spi)			SPI_DISABLE_QUAD_MODE(spi)
#define SPIFLASH_OUTPUT_DIRECTION(spi)		SPI_ENABLE_QUAD_OUTPUT_MODE(spi)
#define SPIFLASH_INPUT_DIRECTION(spi)		SPI_ENABLE_QUAD_INPUT_MODE(spi)
#else //one-bit mode
#define SPIFLASH_READ_CMD					SPIFLASH_FAST_READ
#define SPIFLASH_WRITE_CMD					SPIFLASH_PAGE_PROGRAM
#define SPIFLASH_READ_MODE(spi)						
#define SPIFLASH_WRITE_MODE(spi)
#define SPIFLASH_DEFAULT_MODE(spi)
#define SPIFLASH_OUTPUT_DIRECTION(spi)	
#define SPIFLASH_INPUT_DIRECTION(spi)	
#endif
#endif

#if ((__CHIP_SERIES__==__N570F064_SERIES__)||(__CHIP_SERIES__==__N570H064_SERIES__)||(__CHIP_SERIES__==__N575_SERIES__)|| \
(__CHIP_SERIES__==__N569S_SERIES__)||(__CHIP_SERIES__==__I91000_SERIES__)||(__CHIP_SERIES__==__ISD9000_SERIES__)||(__CHIP_SERIES__==__ISD9100_SERIES__))

#if(defined(SPIFLASH_INTERFACE_MODE)&&(SPIFLASH_INTERFACE_MODE==1))	//Dual mode
#error "The SPI Function DO NOT Support Dual Mode!!!"
#elif(defined(SPIFLASH_INTERFACE_MODE)&&(SPIFLASH_INTERFACE_MODE==2))	//Quad mode
#error "The SPI Function DO NOT Support Quad Mode!!!"
#else //one-bit mode
#define SPIFLASH_READ_CMD					SPIFLASH_FAST_READ
#define SPIFLASH_WRITE_CMD					SPIFLASH_PAGE_PROGRAM
#define SPIFLASH_READ_MODE(spi)						
#define SPIFLASH_WRITE_MODE(spi)
#define SPIFLASH_DEFAULT_MODE(spi)
#define SPIFLASH_OUTPUT_DIRECTION(spi)	
#define SPIFLASH_INPUT_DIRECTION(spi)	
#endif

#endif

#if ((__CHIP_SERIES__==__N572F072__) || (__CHIP_SERIES__==__N572P072_SERIES__) || (__CHIP_SERIES__==__N571P032_SERIES__) || \
(__CHIP_SERIES__==__N572F065_SERIES__) || (__CHIP_SERIES__==__N572F064_SERIES__))


#if(defined(SPIFLASH_INTERFACE_MODE)&&(SPIFLASH_INTERFACE_MODE==1))	//Dual mode
#error "The SPI Function DO NOT Support Dual Mode!!!"
#elif(defined(SPIFLASH_INTERFACE_MODE)&&(SPIFLASH_INTERFACE_MODE==2))	//Quad mode
#error "The SPI Function DO NOT Support Quad Mode!!!"
#else //one-bit mode
#define SPIFLASH_READ_CMD					SPIFLASH_FAST_READ
#define SPIFLASH_WRITE_CMD					SPIFLASH_PAGE_PROGRAM
#define SPIFLASH_READ_MODE(spi)						
#define SPIFLASH_WRITE_MODE(spi)
#define SPIFLASH_DEFAULT_MODE(spi)
#define SPIFLASH_OUTPUT_DIRECTION(spi)	
#define SPIFLASH_INPUT_DIRECTION(spi)	
#endif
#endif

typedef void (*PFN_SPIFLASH_MODE)(SPI_T  *psSpiHandler, UINT32 u32Cmd, UINT32 u32ByteAddr);

typedef struct
{
	SPI_T   *psSpiHandler;				// SPI access handler
#if (SPIFLASH_OPERATION_MODE == 2)
	PFN_SPIFLASH_MODE pfnSPIFlashMode;	// for change 3 or 4 byte address command function
#endif
//#if (SPIFLASH_INTERFACE_MODE == 2)		// for quad mode
	UINT32 pu32MISO1Port;
	UINT32 u32MISO1Pin;
	UINT32 u32MISO1MFP;
//#endif
	UINT32  u32FlashSize;				// SPIFlash memory size
	UINT8   u8SlaveDevice;              // SPIFlash is on device1/2
	UINT8   u8Flag;
	UINT16  u16PageNum;                 // Record Nand flash page to add computing
	UINT8   u8BBTCount;                 // Bad block table max count.
	PUINT16 pau16BBT;					// Bad block table pointer.
} S_SPIFLASH_HANDLER;

typedef enum
{
#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
	eSPIFLASH_STATUS_REG1 = 0xA0,
	eSPIFLASH_STATUS_REG2 = 0xB0,
	eSPIFLASH_STATUS_REG3 = 0xC0
#else
	eSPIFLASH_STATUS_REG1 = 0x00,
	eSPIFLASH_STATUS_REG2 = 0x30,
	eSPIFLASH_STATUS_REG3 = 0x10
#endif
} E_SPIFLASH_STATUS_REGISTER;

// Backward compatible
#define eSTATUS_REG1 eSPIFLASH_STATUS_REG1
#define eSTATUS_REG2 eSPIFLASH_STATUS_REG2
#define eSTATUS_REG3 eSPIFLASH_STATUS_REG3

// APIs declaration
/*******************************************************************/
/*             Miscellaneous API declaration                       */
/*******************************************************************/
void
SPIFlash_Open(
	SPI_T *psSpiHandler,
	UINT8 u8DrvSlaveDevice,
	UINT32 u32SpiClk,
	S_SPIFLASH_HANDLER *psSpiFlashHandler
);

__STATIC_INLINE void
SPIFlash_Close(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
	SPI_Close(psSpiFlashHandler->psSpiHandler);
}

__STATIC_INLINE UINT32
SPIFlash_GetSPIClock(
   S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
	return SPI_GetBusClock(psSpiFlashHandler->psSpiHandler);
}

void
SPIFlash_SendRecOneData(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32Data,
	UINT8  u8DataLen
);

void
SPIFlash_GetChipInfo(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
);

UINT8
SPIFlash_ReadStatusReg(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	E_SPIFLASH_STATUS_REGISTER eStatusReg
);

#if(defined(SPIFLASH_NAND)&&(SPIFLASH_NAND==1))
BOOL
SPIFlash_CreateBBT(
	S_SPIFLASH_HANDLER *psSPIFlashHandler,
	PUINT16 pau16BBTBuffer,
	UINT8 u8BBTCount
);

void
SPIFlash_Erase128K(
	S_SPIFLASH_HANDLER *psSPIFlashHandler,
	UINT16  u16IndexOf128K
);
#else
void
SPIFlash_EN4BAddress(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
);

void
SPIFlash_EX4BAddress(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
);
#endif

void
SPIFlash_WriteStatusRegEx(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	E_SPIFLASH_STATUS_REGISTER eStatusReg,
	UINT16 u16Status,
	UINT8 u8Length
);

//__STATIC_INLINE void
//SPIMFlash_WriteStatusReg(
//	S_SPIFLASH_HANDLER *psSpiFlashHandler,
//	UINT8 u8Status
//)
//{
//	SPIFlash_WriteStatusRegEx(psSpiFlashHandler, eSTATUS_REG1, u8Status, 16);
//}

void
SPIFlash_PowerDown(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	BOOL	bEnable
);

void
SPIFlash_WaitReady(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
);

BOOL
SPIFlash_CheckBusy(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
);

UINT32
SPIFlash_GetVersion(void);

void
SPIFlash_ReadSFDP(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT8 u8ByteAddr,
	UINT8 u8ByteLen
);

UINT32
SPIFlash_GetJedecID(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
);

void SPIFlash_WaitStable(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32MaxReadVerifyCount
);

__STATIC_INLINE void
SPIFlash_Dummy(
	SPI_T *psSpiHandler
)
{
	
}

/*******************************************************************/
/*             Protection API declaration                          */
/*******************************************************************/
void
SPIFlash_ChipWriteEnable(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	BOOL bEnableWrite
);

void
SPIFlash_GlobalProtect(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	BOOL bEnableGlobalProtect
);

/*******************************************************************/
/*             Write API declaration                               */
/*******************************************************************/
void
SPIFlash_Write(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32Addr,
	PUINT8 pau8Data,
	UINT32 u32DataLen
);

void
SPIFlash_WritePage(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32PageAddr,
	PUINT8 pau8Data
);

void
SPIFlash_WriteStart(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32ByteAddr
);

void
SPIFlash_WriteEnd(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
);

UINT32
SPIFlash_WriteData(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32SPIAddr,
	PUINT8 pau8Data,
	UINT32 u32DataLen
);

void
SPIFlash_WriteDataAlign(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	PUINT8 pau8Data
);

/*******************************************************************/
/*             Read API declaration                                */
/*******************************************************************/
void
SPIFlash_Read(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32ByteAddr,
	PUINT8 pau8Data,
	UINT32 u32DataLen
);

void
SPIFlash_BurstRead(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32ByteAddr,
	PUINT8 pau8Data,
	UINT32 u32DataLen
);

void
SPIFlash_ReadStart(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32ByteAddr
);

void
SPIFlash_ReadEnd(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
);

void
SPIFlash_ReadData(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	PUINT8 pau8Data,
	UINT32 u32DataLen
);

void
SPIFlash_ReadDataAlign(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	PUINT8 pau8Data,
	UINT32 u32DataLen
);

void SPIFlash_ReadQuadIO(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT32 u32StartAddr,
	PUINT8 pau8Data,
	UINT32 u32DataLen
);

/*******************************************************************/
/*             Erase API declaration                               */
/*******************************************************************/
/********* none blocking releate APIs *********/
void
SPIFlash_EraseStart(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT8 u8Cmd,
	UINT32 u32Addr
);

__STATIC_INLINE void
SPIFlash_Erase64KStart(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT16  u16IndexOf64K
)
{
	SPIFlash_EraseStart(psSpiFlashHandler, SPIFLASH_64K_ERASE, (u16IndexOf64K<<16));
}

__STATIC_INLINE void
SPIFlash_Erase4KStart(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT16 u16IndexOf4K
)
{
	SPIFlash_EraseStart(psSpiFlashHandler, SPIFLASH_4K_ERASE, (u16IndexOf4K<<12));
}

__STATIC_INLINE void
SPIFlash_Erase32KStart(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT16 u16IndexOf32K
)
{
	SPIFlash_EraseStart(psSpiFlashHandler, SPIFLASH_32K_ERASE, (u16IndexOf32K<<15));
}

__STATIC_INLINE void
SPIFlash_EraseChipStart(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
	SPIFlash_ChipWriteEnable(psSpiFlashHandler, TRUE);
	SPIFlash_SendRecOneData(psSpiFlashHandler,SPIFLASH_CHIP_ERASE,8);
}

/********* blocking releate APIs *********/
__STATIC_INLINE void
SPIFlash_Erase(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT8 u8Cmd,
	UINT32 u32Addr
)
{
	SPIFlash_EraseStart(psSpiFlashHandler, u8Cmd, u32Addr);
	// Wait erase complete
	SPIFlash_WaitReady(psSpiFlashHandler);
}

__STATIC_INLINE void
SPIFlash_Erase64K(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT16  u16IndexOf64K
)
{
	SPIFlash_Erase(psSpiFlashHandler, SPIFLASH_64K_ERASE, (u16IndexOf64K<<16));
}

__STATIC_INLINE void
SPIFlash_Erase4K(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT16 u16IndexOf4K
)
{
	SPIFlash_Erase(psSpiFlashHandler, SPIFLASH_4K_ERASE, (u16IndexOf4K<<12));
}

__STATIC_INLINE void
SPIFlash_Erase32K(
	S_SPIFLASH_HANDLER *psSpiFlashHandler,
	UINT16 u16IndexOf32K
)
{
	SPIFlash_Erase(psSpiFlashHandler, SPIFLASH_32K_ERASE, (u16IndexOf32K<<15));
}

__STATIC_INLINE void
SPIFlash_EraseChip(
	S_SPIFLASH_HANDLER *psSpiFlashHandler
)
{
	SPIFlash_EraseChipStart(psSpiFlashHandler);
	// Wait erase complete
	SPIFlash_WaitReady(psSpiFlashHandler);
}

/*******************************************************************/
/*             Quad Mode Eanbel API By Manufacturer                */
/*******************************************************************/
#if ((__CHIP_SERIES__ == __NPCA121_SERIES__) || (__CHIP_SERIES__ == __I94100_SERIES__))
__STATIC_INLINE void
SPIFlash_QuadMode_W25Q16BV(S_SPIFLASH_HANDLER *psSpiFlashHandler)
{
	UINT16 u16Status;
	u16Status = SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG1) | (SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG2) << 8);
	SPIFlash_WriteStatusRegEx(psSpiFlashHandler, eSTATUS_REG1, u16Status|0x0200, 24);           
}

__STATIC_INLINE void
SPIFlash_QuadMode_W25Q16DVS(S_SPIFLASH_HANDLER *psSpiFlashHandler, BOOL bEn)
{
	UINT16 u16Status;
    u16Status = SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG1) | (SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG2) << 8);
	if (bEn)
		SPIFlash_WriteStatusRegEx(psSpiFlashHandler, eSTATUS_REG1, u16Status|0x0200, 24);
	else	
		SPIFlash_WriteStatusRegEx(psSpiFlashHandler, eSTATUS_REG1, u16Status&0xfdff, 24);
}

__STATIC_INLINE void
SPIFlash_QuadMode_W25Q64DW(S_SPIFLASH_HANDLER *psSpiFlashHandler)
{
	SPIFlash_QuadMode_W25Q16BV(psSpiFlashHandler);
}

__STATIC_INLINE void
SPIFlash_QuadMode_W25Q80BV(S_SPIFLASH_HANDLER *psSpiFlashHandler)
{
	SPIFlash_QuadMode_W25Q16BV(psSpiFlashHandler);
}

__STATIC_INLINE void
SPIFlash_QuadMode_GD25Q16C(S_SPIFLASH_HANDLER *psSpiFlashHandler)
{
	SPIFlash_QuadMode_W25Q16BV(psSpiFlashHandler);
}

__STATIC_INLINE void
SPIFlash_QuadMode_W25Q16FW(S_SPIFLASH_HANDLER *psSpiFlashHandler)
{
	UINT8 u8Status;
	u8Status = SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG2);
	SPIFlash_WriteStatusRegEx(psSpiFlashHandler, eSTATUS_REG2, (UINT16)(u8Status|0x02), 16);
}

__STATIC_INLINE void
SPIFlash_QuadMode_W25Q256FV(S_SPIFLASH_HANDLER *psSpiFlashHandler)
{
	UINT8 u8Status;

	u8Status = SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG2);
		
	SPIFlash_ChipWriteEnable(psSpiFlashHandler, TRUE);
	// Status2 Bit-1(S9): Quad-mode enable.
	SPIFlash_SendRecOneData(psSpiFlashHandler,((SPIFLASH_WRITE_STATUS|eSTATUS_REG2)<<8)|u8Status|0x02, 16);

	SPIFlash_WaitReady(psSpiFlashHandler);
}

__STATIC_INLINE void
SPIFlash_QuadMode_W25Q64FV(S_SPIFLASH_HANDLER *psSpiFlashHandler)
{
	UINT8 u8Status1;
	UINT8 u8Status2;
	u8Status1 = SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG1);
	u8Status2 = SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG2);
		
	SPIFlash_ChipWriteEnable(psSpiFlashHandler, TRUE);
	// Status2 Bit-1(S9): Quad-mode enable.

	SPIFlash_SendRecOneData(psSpiFlashHandler,((SPIFLASH_WRITE_STATUS<<16)|(u8Status1<<8)|(u8Status2|0x2)), 24);

	SPIFlash_WaitReady(psSpiFlashHandler);
}

__STATIC_INLINE void
SPIFlash_QuadMode_EN25QH256(S_SPIFLASH_HANDLER *psSpiFlashHandler)
{
	UINT8 u8Status;
	u8Status = SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG1);
	
	//SPIFlash_WriteStatusRegEx(psSpiFlashHandler, eSTATUS_REG2, (UINT16)(u8Status|0x02), 16);
	
	SPIFlash_ChipWriteEnable(psSpiFlashHandler, TRUE);
	// Status2 Bit-2: Quad-mode enable.
	SPIFlash_SendRecOneData(psSpiFlashHandler,((SPIFLASH_WRITE_STATUS|eSTATUS_REG1)<<8)|u8Status|0x20, 16);
	SPIFlash_WaitReady(psSpiFlashHandler);
	
	// Check if the QE bit is enable.
	u8Status = SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG1);
	if(u8Status & 0x20)
	{
		// If the SPI Flash quad-mode isenable, set the MFP back.
		#if ((__CHIP_SERIES__ == __NPCA121_SERIES__) || (__CHIP_SERIES__ == __I94100_SERIES__))
		uint32_t *pu32Port;
		pu32Port = (uint32_t *)(psSpiFlashHandler->pu32MISO1Port);
		*pu32Port = (*pu32Port & ~(psSpiFlashHandler->u32MISO1Pin)) | psSpiFlashHandler->u32MISO1MFP;
		#endif
	}
}

__STATIC_INLINE void
SPIFlash_QuadMode_MX25L3235E(S_SPIFLASH_HANDLER *psSpiFlashHandler)
{
	UINT8 u8Status;
	u8Status = SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG1);
	SPIFlash_WriteStatusRegEx(psSpiFlashHandler, eSTATUS_REG1, (u8Status|0x40)<<8, 24);
}
#endif
#if ((__CHIP_SERIES__ == __ISD9300_SERIES__) || (__CHIP_SERIES__ == __I91200_SERIES__) || (__CHIP_SERIES__ == __I91200BS_SERIES__))
__STATIC_INLINE void
SPIFlash_QuadMode_W25Q16BV(S_SPIFLASH_HANDLER *psSpiFlashHandler, BOOL bEn)
{
	UINT16 u16Status;
    u16Status = (SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG1) << 8) | SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG2);
	if (bEn)
		SPIFlash_WriteStatusRegEx(psSpiFlashHandler, eSTATUS_REG1, u16Status|0x02, 24);
	else	
		SPIFlash_WriteStatusRegEx(psSpiFlashHandler, eSTATUS_REG1, u16Status&0xfffd, 24);
}

__STATIC_INLINE void
SPIFlash_QuadMode_W25Q16DVS(S_SPIFLASH_HANDLER *psSpiFlashHandler, BOOL bEn)
{
	UINT16 u16Status;
    u16Status = (SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG1) << 8) | SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG2);
	if (bEn)
		SPIFlash_WriteStatusRegEx(psSpiFlashHandler, eSTATUS_REG1, u16Status|0x02, 24);
	else	
		SPIFlash_WriteStatusRegEx(psSpiFlashHandler, eSTATUS_REG1, u16Status&0xfffd, 24);
}

__STATIC_INLINE void
SPIFlash_QuadMode_W25Q64DW(S_SPIFLASH_HANDLER *psSpiFlashHandler, BOOL bEn)
{
	SPIFlash_QuadMode_W25Q16BV(psSpiFlashHandler, bEn);
}

__STATIC_INLINE void
SPIFlash_QuadMode_W25Q80BV(S_SPIFLASH_HANDLER *psSpiFlashHandler, BOOL bEn)
{
	SPIFlash_QuadMode_W25Q16BV(psSpiFlashHandler, bEn);
}

__STATIC_INLINE void
SPIFlash_QuadMode_GD25Q16C(S_SPIFLASH_HANDLER *psSpiFlashHandler, BOOL bEn)
{
	SPIFlash_QuadMode_W25Q16BV(psSpiFlashHandler, bEn);
}

__STATIC_INLINE void
SPIFlash_QuadMode_W25Q16FW(S_SPIFLASH_HANDLER *psSpiFlashHandler, BOOL bEn)
{
	UINT8 u8Status;
	u8Status = SPIFlash_ReadStatusReg(psSpiFlashHandler, eSTATUS_REG2);
	if (bEn)
		SPIFlash_WriteStatusRegEx(psSpiFlashHandler, eSTATUS_REG2, (UINT16)(u8Status|0x02), 16);
	else
		SPIFlash_WriteStatusRegEx(psSpiFlashHandler, eSTATUS_REG2, (UINT16)(u8Status&0xfd), 16);
}

__STATIC_INLINE void
SPIFlash_QuadMode_W25Q128JV(S_SPIFLASH_HANDLER *psSpiFlashHandler, BOOL bEn)
{
	SPIFlash_QuadMode_W25Q16FW(psSpiFlashHandler, bEn);       
}
#endif
#ifdef  __cplusplus
}
#endif

#endif	// __SPIFLASH_H__

