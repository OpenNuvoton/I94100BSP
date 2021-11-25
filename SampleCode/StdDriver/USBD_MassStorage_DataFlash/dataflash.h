

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_RW_H
#define __FLASH_RW_H

#include "stdint.h"
 
	

/*----------------------------------------------------------------------------------*/

void DataFlashRead(uint32_t addr, uint32_t size, uint32_t buffer);
void DataFlashReadPage(uint32_t addr, uint32_t buffer, uint32_t len);
uint32_t DataFlashProgramPage(uint32_t u32StartAddr, uint32_t * u32Buf, uint32_t len);
void DataFlashWrite(uint32_t addr, uint32_t size, uint32_t buffer);

void MSC_SetStallEP (uint32_t EPNum);
uint32_t MSC_Reset (void);
void MSC_MemoryRead (void);
void MSC_MemoryWrite (void);
void MSC_MemoryVerify (void);
	

#endif /* __FLASH_RW_H */
