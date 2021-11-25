/******************************************************************************
 * @file     DataFlash_RW.c
 * @brief    
 *
 * @note
 * Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

/*!<Includes */
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "massstorage.h"
#include "usbd_bot.h"
#include "dataflash.h"
#include "fmc.h"


uint32_t g_sectorBuf[FLASH_PAGE_SIZE / 4];
volatile uint8_t flash_sectors_manager[16][8] = {0};

////////////////////////////////////////////////////////////////////
void DataFlashRead(uint32_t addr, uint32_t size, uint32_t buffer)
{
    /* This is low level read function of USB Mass Storage */
    int32_t len;
    uint32_t i;
    uint32_t * pu32Buf = (uint32_t *)buffer;

    /* Modify the address to MASS_STORAGE_OFFSET */
    addr += MASS_STORAGE_OFFSET;
    len = (int32_t)size;

    while(len >= BUFFER_PAGE_SIZE)
    {
        //FMC_ReadPage(addr, (uint32_t *)buffer);
        for(i = 0; i < BUFFER_PAGE_SIZE / 4; i++)
            pu32Buf[i] = FMC_Read(addr + i * 4);
        addr   += BUFFER_PAGE_SIZE;
        buffer += BUFFER_PAGE_SIZE;
        len  -= BUFFER_PAGE_SIZE;
        pu32Buf = (uint32_t *)buffer;
    }
}

////////////////////////////////////////////////////////////////////
void DataFlashReadPage(uint32_t addr, uint32_t buffer, uint32_t len)
{
    uint32_t i;	
    uint32_t * pu32Buf = (uint32_t *)buffer;

			for(i = 0; i < len / 4; i++)
        pu32Buf[i] = FMC_Read(addr + i * 4);
}


////////////////////////////////////////////////////////////////////
uint32_t DataFlashProgramPage(uint32_t u32StartAddr, uint32_t * u32Buf, uint32_t len)
{
    uint32_t i;
	
	  for(i = 0; i < len / 4; i++)
    {
        FMC_Write(u32StartAddr + i * 4, u32Buf[i]);
    }

    return 0;
}

void DataFlashWrite(uint32_t addr, uint32_t size, uint32_t buffer)
{
    /* This is low level write function of USB Mass Storage */
    int32_t len, i, offset;
    uint32_t *pu32;
    uint32_t alignAddr;	
	  uint8_t  page_num, sect_num;

	
    /* Modify the address to MASS_STORAGE_OFFSET */
    addr += MASS_STORAGE_OFFSET;

    len = (int32_t)size;

    if((len == FLASH_PAGE_SIZE) && ((addr & (FLASH_PAGE_SIZE - 1)) == 0))
    {
			  
        /* Page erase */
        FMC_Erase(addr);
			 
        while(len >= FLASH_PAGE_SIZE)
        {
            DataFlashProgramPage(addr, (uint32_t *) buffer, FLASH_PAGE_SIZE);
            len    -= FLASH_PAGE_SIZE;
            buffer += FLASH_PAGE_SIZE;
            addr   += FLASH_PAGE_SIZE;
        }
    }
    else
    {
        do
        {   
						alignAddr = addr & 0x1F000;		
            page_num = ((alignAddr & 0xf000) >> 12);  
            sect_num = ((addr & 0xf00 ) >> 9 );       				
					
						if ( flash_sectors_manager[page_num][sect_num] )		
						{
								/* Get the sector offset*/
								offset = (addr & (FLASH_PAGE_SIZE - 1));
								
 								/* Get the update length */
 								len = FLASH_PAGE_SIZE - offset;
 								if( len >= size )
 										len = size;
								
								/* Write to the destination sector */
								pu32 = (uint32_t *)buffer;
								DataFlashProgramPage(addr, (uint32_t *) pu32, len);		
								flash_sectors_manager[page_num][sect_num] = 0;	
						}
						else
						{
								/* Get the sector offset*/
								offset = (addr & (FLASH_PAGE_SIZE - 1));	
							
								/* Read flash data */							
							  DataFlashReadPage(alignAddr, (uint32_t)&g_sectorBuf[0], FLASH_PAGE_SIZE);			
							
								/* Source buffer */
								pu32 = (uint32_t *)buffer;
								
								/* Get the update length */
								len = FLASH_PAGE_SIZE - offset;
								if( len >= size )
										len = size;
								
								/* Update the destination buffer */
								for(i = 0; i < len / 4; i++){
									g_sectorBuf[offset / 4 + i] = pu32[i];
								}
								
								/* Page erase */
								FMC_Erase(alignAddr);
								DataFlashProgramPage(alignAddr, (uint32_t *) &g_sectorBuf[0], FLASH_PAGE_SIZE);
								for ( i = 0 ; i < 8 ; i++){
									flash_sectors_manager[page_num][i] = 0;
								}
							
						}
						
            size -= len;
            addr += len;
            buffer += len;

        }
        while(size > 0);
    }
}


//////////////////////////////////////////////////////////////////
/*
 *  Set Stall for MSC Endpoint
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *    Return Value:    None
 */

void MSC_SetStallEP (uint32_t EPNum) 
{          	
		EPNum &= 0x7F;		  
		USBD_SET_EP_STALL(EPNum);
}


/*
 *  MSC Mass Storage Reset Request Callback
 *   Called automatically on Mass Storage Reset Request
 *    Parameters:      None (global SetupPacket and EP0Buf)
 *    Return Value:    TRUE - Success, FALSE - Error
 */

uint32_t MSC_Reset (void) 
{  
	g_u32Offset = g_u32Length = 0;
	g_u8BotState = BOT_IDLE;
	g_sCBW.dSignature = BOT_CBW_SIGNATURE;
  return (TRUE);
}



/*
 *  MSC Memory Read Callback
 *   Called automatically on Memory Read Event
 *    Parameters:      None (global variables)
 *    Return Value:    None
 */

void MSC_MemoryRead (void) 
{
	uint8_t *pu8Src, *pu8Dst;
	uint32_t i, pu32Buf[64/4];
  uint32_t Transmit_Len;
	
	Transmit_Len = GetMin(g_u32Length, MSC_MAX_PACKET);
	

	// Read flash data 64B each time
	for(i = 0; i < Transmit_Len / 4; i++)
	{
		pu32Buf[i] = FMC_Read(MASS_STORAGE_OFFSET + g_u32Offset + i * 4);	// Offset means address 
	}
	
	pu8Src = (uint8_t *)&pu32Buf;
	pu8Dst = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(BULK_IN_EP));
	
  for(i = 0 ; i < Transmit_Len ;i++){
		pu8Dst[i] = pu8Src[i]; 
	}	
	
	USBD_SET_PAYLOAD_LEN(BULK_IN_EP, Transmit_Len);
	
  g_u32Offset += Transmit_Len;
  g_u32Length -= Transmit_Len;

  g_sCSW.dDataResidue -= Transmit_Len;

  if ( g_u32Length == 0 ) 	
	{

    g_u8BotState = BOT_DATA_IN_LAST;
  }

  if (g_u8BotState != BOT_DATA_IN) 
	{
    g_sCSW.bStatus = CSW_CMD_PASSED;
  }
}

/*
 *  MSC Memory Write Callback
 *   Called automatically on Memory Write Event
 *    Parameters:      None (global variables)
 *    Return Value:    None
 */

void MSC_MemoryWrite (void) 	
{
  uint32_t i;


  for (i = 0; i < g_u8BulkLen; i++) 
	{
    g_au8Memory[g_u32Offset + i] = g_au8BulkBuf[i];				
  }
	
  g_u32Offset += g_u8BulkLen;
  g_u32Length -= g_u8BulkLen;

  g_sCSW.dDataResidue -= g_u8BulkLen;
	
	USBD_SET_PAYLOAD_LEN(BULK_OUT_EP, EP3_MAX_PKT_SIZE);    
	
	/* 4KB Buffer full. Writer it to storage first. */
	if ( (g_u32Offset - g_u32DataFlashStartAddr) == UDC_SECTOR_SIZE )
	{
			DataFlashWrite(g_u32DataFlashStartAddr, UDC_SECTOR_SIZE, (uint32_t)&g_au8Memory[g_u32DataFlashStartAddr] /*STORAGE_DATA_BUF*/);
		  g_u32DataFlashStartAddr = g_u32Offset;		
	}
	
  if ((g_u32Length == 0) || (g_u8BotState == BOT_CSW_Send)) 
	{	
		Set_Pkt_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
  }
	
}


/*
 *  MSC Memory Verify Callback
 *   Called automatically on Memory Verify Event
 *    Parameters:      None (global variables)
 *    Return Value:    None
 */

void MSC_MemoryVerify (void) 
{
  uint32_t n;
	uint8_t CSW_Status;

  if ((g_u32Offset + g_u8BulkLen) > DATA_FLASH_STORAGE_SIZE) {
    g_u8BulkLen = DATA_FLASH_STORAGE_SIZE - g_u32Offset;
    g_u8BotState = BOT_CSW_Send;
    MSC_SetStallEP(BULK_OUT_EP);		
  }

  for (n = 0; n < g_u8BulkLen; n++) 
	{
    if (g_au8Memory[g_u32Offset + n] != g_au8BulkBuf[n]) 
		{
      g_u32MemOK = FALSE;
      break;
    }
  }

  g_u32Offset += g_u8BulkLen;
  g_u32Length -= g_u8BulkLen;

  g_sCSW.dDataResidue -= g_u8BulkLen;

  if ((g_u32Length == 0) || (g_u8BotState == BOT_CSW_Send)) 
	{
		CSW_Status = (g_u32MemOK) ? CSW_CMD_PASSED : CSW_CMD_FAILED;
		Set_Pkt_CSW (CSW_Status, SEND_CSW_ENABLE);
  }
}



