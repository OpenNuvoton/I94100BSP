/******************************************************************************
 * @file     usbd_bot.c
 * @brief    Bulk-Only Transport Parser
 *
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "massstorage.h"
#include "usbd_bot.h"
#include "usbd_scsi.h"
#include "dataflash.h"

SCSI_COMMAND_REQUESTS  scsi_command_requests = 
{
	SCSI_TestUnitReady_Cmnd,
	SCSI_RequestSense_Cmnd,
	SCSI_Inquiry_Cmnd,
	SCSI_Start_Stop_Unit_Cmnd,
	SCSI_ModeSense6_Cmnd,
	SCSI_ModeSense10_Cmnd,
	SCSI_ReadFormatCapacity_Cmnd,
	SCSI_ReadCapacity10_Cmnd,
	SCSI_ReadCapacity16_Cmnd,
	SCSI_Write10_Cmnd,
	SCSI_Read10_Cmnd,
	SCSI_Verify10_Cmnd,
	SCSI_Format_Cmnd,
	/*Unsupported command*/
	SCSI_Send_Diagnostic_Cmnd, 
	SCSI_Mode_Select10_Cmnd,
	SCSI_Mode_Select6_Cmnd,	
	SCSI_Read6_Cmnd,
	SCSI_Read16_Cmnd,
	SCSI_Write6_Cmnd,
	SCSI_Write16_Cmnd,
	SCSI_Verify12_Cmnd,
	SCSI_Verify16_Cmnd
};

SCSI_COMMAND_REQUESTS *SCSI_Command_Request = &scsi_command_requests;


Bulk_Only_CBW 	g_sCBW;                  /* Command Block Wrapper */
Bulk_Only_CSW 	g_sCSW;                  /* Command Status Wrapper */

uint8_t   g_u8MaxLun=0;
uint8_t   g_u8BotState;
uint8_t   g_au8Memory[DATA_FLASH_STORAGE_SIZE];  
uint32_t  g_u32MemOK;                   				 
uint32_t  g_u32Offset;                  				 
uint32_t  g_u32Length;                  				 
uint8_t  	g_u8BulkLen;                 					 
uint8_t  	g_au8BulkBuf[MSC_MAX_PACKET]; 				 
uint32_t  g_u32DataFlashStartAddr;



/*
 *  Write USB Endpoint Data
 *    Parameters:      EPNum: Endpoint Number
 *                       EPNum.0..3: Address
 *                       EPNum.7:    Dir
 *                     pData: Pointer to Data Buffer
 *                     cnt:   Number of bytes to write
 *    Return Value:    Number of bytes written
 */

uint32_t USB_WriteEP (uint32_t EPNum, uint8_t *pSrc, uint32_t byte_cnt) 
{	
	uint8_t *pDest;
	uint8_t i;
	
	EPNum &= 0x7F;		
	
	pDest = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EPNum));
  for(i = 0 ; i < byte_cnt ;i++)
	{
		pDest[i] = pSrc[i]; 
	}	
	
  USBD_SET_PAYLOAD_LEN(EPNum, byte_cnt);  
	return (TRUE);
	
}



/*******************************************************************************
* Function Name  : Bot_Abort
* Description    : Stall the needed Endpoint according to the selected direction.
* Input          : Endpoint direction IN, OUT or both directions
* Output         : None.
* Return         : None.
*******************************************************************************/
void Bot_Abort(uint8_t Direction)
{
  switch (Direction)
  {
    case DIR_IN :
		  MSC_SetStallEP(BULK_IN_EP);
      break;
    case DIR_OUT :
		  MSC_SetStallEP(BULK_OUT_EP);
      break;
    case BOTH_DIR :
		  MSC_SetStallEP(BULK_IN_EP);
		  MSC_SetStallEP(BULK_OUT_EP);
      break;
    default:
      break;
  }
}

/*******************************************************************************
* Function Name  : Set_Pkt_CSW
* Description    : Set the SCW with the needed fields.
* Input          : uint8_t CSW_Status this filed can be CSW_CMD_PASSED,CSW_CMD_FAILED,
*                  or CSW_PHASE_ERROR.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Set_Pkt_CSW (uint8_t CSW_Status, uint8_t Send_Permission)
{
	uint8_t *pu8Src, *pu8Dst;
	uint8_t i;
	
  g_sCSW.dSignature = BOT_CSW_SIGNATURE;
  g_sCSW.bStatus = CSW_Status;

	pu8Src = (uint8_t *)& g_sCSW;
	pu8Dst = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(BULK_IN_EP));
	
  for(i = 0 ; i < CSW_DATA_LENGTH ;i++){
		pu8Dst[i] = pu8Src[i]; 
	}	

  g_u8BotState = BOT_ERROR;
	
  if (Send_Permission)
  {
    g_u8BotState = BOT_CSW_Send;
		USBD_SET_PAYLOAD_LEN(BULK_IN_EP, CSW_DATA_LENGTH);   
		
  }

}

/*******************************************************************************
* Function Name  : Transfer_Data_Request
* Description    : Send the request response to the PC HOST.
* Input          : uint8_t* Data_Address : point to the data to transfer.
*                  uint16_t Data_Length : the nember of Bytes to transfer.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Transfer_Data_Request(uint8_t* Data_Pointer, uint16_t Data_Len, uint8_t Enable_Stall)
{
	uint8_t *pu8;
	uint8_t i;
	
	pu8 = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(BULK_IN_EP));

  for(i = 0 ; i < Data_Len ;i++)
	{
		pu8[i] = Data_Pointer[i]; 
	}	
	
	USBD_SET_PAYLOAD_LEN(BULK_IN_EP, Data_Len);
	
	if (Enable_Stall) 
	{
		g_u8BotState = BOT_DATA_IN_LAST_STALL;
	}	
	else 
	{
		g_u8BotState = BOT_DATA_IN_LAST;
	}	
		
  g_sCSW.dDataResidue -= Data_Len;
  g_sCSW.bStatus = CSW_CMD_PASSED;
}


/*******************************************************************************
* Function Name  : CBW_Pkt_Decode
* Description    : Decode the received CBW and call the related SCSI command
*                 routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void CBW_Pkt_Decode(void)		
{
  uint32_t n;
	
	USBD_MemCopy((uint8_t *)&g_sCBW, &g_au8BulkBuf[0], g_u8BulkLen);
	
  g_sCSW.dTag = g_sCBW.dTag;
  g_sCSW.dDataResidue = g_sCBW.dDataLength;
	
  if (g_u8BulkLen != BOT_CBW_PACKET_LENGTH)
  {
			Bot_Abort(BOTH_DIR);
			USBD_LockEpStall(BIT2|BIT3);					
    
			Set_Scsi_Sense_Data(g_sCBW.bLUN, ILLEGAL_REQUEST, PARAMETER_LIST_LENGTH_ERROR);
			Set_Pkt_CSW(CSW_CMD_FAILED, SEND_CSW_DISABLE);
			return;
  }

  if ((g_sCBW.CB[0] == SCSI_READ10) || (g_sCBW.CB[0] == SCSI_WRITE10))
  {
			n = (g_sCBW.CB[2] << 24) | (g_sCBW.CB[3] << 16) | (g_sCBW.CB[4] <<  8) | g_sCBW.CB[5];
			g_u32Offset = n * UDC_SECTOR_SIZE;
			g_u32DataFlashStartAddr = g_u32Offset;		
			
			n = (g_sCBW.CB[7] <<  8) | g_sCBW.CB[8];
			g_u32Length = n * UDC_SECTOR_SIZE;		
  }
	else if ((g_sCBW.CB[0] == SCSI_READ12) || (g_sCBW.CB[0] == SCSI_WRITE12))
	{
			n = (g_sCBW.CB[2] << 24) | (g_sCBW.CB[3] << 16) | (g_sCBW.CB[4] <<  8) | g_sCBW.CB[5];
			g_u32Offset = n * UDC_SECTOR_SIZE;
			g_u32DataFlashStartAddr = g_u32Offset;		
			
			n = (g_sCBW.CB[6] << 24) | (g_sCBW.CB[7] << 16) | (g_sCBW.CB[8] <<  8) | g_sCBW.CB[9];
			g_u32Length = n * UDC_SECTOR_SIZE;		
	}

  if (g_sCBW.dSignature == BOT_CBW_SIGNATURE)
  {		
			/* Valid CBW */
			if ((g_sCBW.bLUN > g_u8MaxLun) || (g_sCBW.bCBLength < 1) || (g_sCBW.bCBLength > 16))
			{			
				Bot_Abort(BOTH_DIR);
				Set_Scsi_Sense_Data(g_sCBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
				Set_Pkt_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
			}
			else
			{
					switch (g_sCBW.CB[0])
					{
							case SCSI_TEST_UNIT_READY:
								SCSI_Command_Request->TestUnitReady_Cmnd(g_sCBW.bLUN);		
								break;
							case SCSI_REQUEST_SENSE:
								SCSI_Command_Request->RequestSense_Cmnd(g_sCBW.bLUN);
								break;
							case SCSI_INQUIRY:
								SCSI_Command_Request->Inquiry_Cmnd(g_sCBW.bLUN);
								break;
							case SCSI_START_STOP_UNIT:
								SCSI_Command_Request->Start_Stop_Unit_Cmnd(g_sCBW.bLUN);
								break;
							case SCSI_ALLOW_MEDIUM_REMOVAL:
								SCSI_Command_Request->Start_Stop_Unit_Cmnd(g_sCBW.bLUN);
								break;
							case SCSI_MODE_SENSE6:
								SCSI_Command_Request->ModeSense6_Cmnd(g_sCBW.bLUN);
								break;
							case SCSI_MODE_SENSE10:
								SCSI_Command_Request->ModeSense10_Cmnd(g_sCBW.bLUN);
								break;
							case SCSI_READ_FORMAT_CAPACITIES:
								SCSI_Command_Request->ReadFormatCapacity_Cmnd(g_sCBW.bLUN);
								break;
							case SCSI_READ_CAPACITY10:
								SCSI_Command_Request->ReadCapacity10_Cmnd(g_sCBW.bLUN);
								break;
							case SCSI_READ_CAPACITY16:
								SCSI_Command_Request->ReadCapacity16_Cmnd(g_sCBW.bLUN);
								break;							
							case SCSI_WRITE10:
							case SCSI_WRITE12:	
								SCSI_Command_Request->Write10_Cmnd(g_sCBW.bLUN);
								break;
							case SCSI_READ10:
							case SCSI_READ12:	
							  SCSI_Command_Request->Read10_Cmnd(g_sCBW.bLUN);
								break;
							case SCSI_VERIFY10:
								SCSI_Command_Request->Verify10_Cmnd(g_sCBW.bLUN);
								break;
							case SCSI_FORMAT_UNIT:
								SCSI_Command_Request->Format_Cmnd(g_sCBW.bLUN);
								break;
							
							//////////////////////////////////////////////////////////
						 /*Unsupported command*/
							case SCSI_MODE_SELECT10:
								SCSI_Command_Request->Mode_Select10_Cmnd(g_sCBW.bLUN);
								break;
							case SCSI_MODE_SELECT6:
								SCSI_Command_Request->Mode_Select6_Cmnd(g_sCBW.bLUN);
								break;
							case SCSI_SEND_DIAGNOSTIC:
								SCSI_Command_Request->Send_Diagnostic_Cmnd(g_sCBW.bLUN);
								break;
							case SCSI_READ6:
								SCSI_Command_Request->Read6_Cmnd(g_sCBW.bLUN);
								break;
							case SCSI_READ16:
								SCSI_Command_Request->Read16_Cmnd(g_sCBW.bLUN);
								break;	
							case SCSI_WRITE6:
								SCSI_Command_Request->Write6_Cmnd(g_sCBW.bLUN);
								break;
							case SCSI_WRITE16:
								SCSI_Command_Request->Write16_Cmnd(g_sCBW.bLUN);
								break;
							case SCSI_VERIFY12:
								SCSI_Command_Request->Verify12_Cmnd(g_sCBW.bLUN);
								break;
							case SCSI_VERIFY16:
								SCSI_Command_Request->Verify16_Cmnd(g_sCBW.bLUN);
								break;

							default:
							{
								Bot_Abort(BOTH_DIR);
								Set_Scsi_Sense_Data(g_sCBW.bLUN, ILLEGAL_REQUEST, INVALID_COMMAND);
								Set_Pkt_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
							}
					}
			}
  }
  else
  {	
     /* Invalid CBW */
     Bot_Abort(BOTH_DIR);
		 USBD_LockEpStall(BIT2|BIT3);		
		
     Set_Scsi_Sense_Data(g_sCBW.bLUN, ILLEGAL_REQUEST, INVALID_COMMAND);
     Set_Pkt_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
  }
	
}



/*
 *  MSC Bulk In Callback
 *    Parameters:      None (global variables)
 *    Return Value:    None
 */

void Mass_Storage_In (void) 
{
		switch (g_u8BotState)
  {
			case BOT_CSW_Send:
			case BOT_ERROR:
					g_u8BotState = BOT_IDLE;
			    USBD_SET_PAYLOAD_LEN(BULK_OUT_EP, EP3_MAX_PKT_SIZE);   
      break;
			case BOT_DATA_IN:		
					switch (g_sCBW.CB[0])
					{
						case SCSI_READ10:
						case SCSI_READ12:	
							SCSI_Read10_Cmnd(g_sCBW.bLUN);  
						break;
					}
			break;
			case BOT_DATA_IN_LAST:	
					Set_Pkt_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
			    USBD_SET_PAYLOAD_LEN(BULK_OUT_EP, EP3_MAX_PKT_SIZE);   
      break;
			case BOT_DATA_IN_LAST_STALL: 
					MSC_SetStallEP(BULK_IN_EP);					
					Set_Pkt_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
			    USBD_SET_PAYLOAD_LEN(BULK_OUT_EP, EP3_MAX_PKT_SIZE);   
      break;
    default:
      break;
		
  }
}


/*
 *  MSC Bulk Out Callback
 *    Parameters:      None (global variables)
 *    Return Value:    None
 */

void Mass_Storage_Out (void) 	
{
	uint8_t *pData;
	uint8_t CMD;
	
  CMD = g_sCBW.CB[0];
	 
	pData = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(BULK_OUT_EP));
  g_u8BulkLen = USBD_GET_PAYLOAD_LEN(BULK_OUT_EP);
	USBD_MemCopy(&g_au8BulkBuf[0], pData, g_u8BulkLen);

  switch (g_u8BotState)
  {
    case BOT_IDLE:
      CBW_Pkt_Decode();
      break;
    case BOT_DATA_OUT:		
      if ( (CMD == SCSI_WRITE10) || (CMD == SCSI_WRITE12) ) 
      {
        SCSI_Write10_Cmnd(g_sCBW.bLUN);
        break;
      }		
      Bot_Abort(DIR_OUT);
      Set_Scsi_Sense_Data(g_sCBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
      Set_Pkt_CSW (CSW_PHASE_ERROR, SEND_CSW_DISABLE);
      break;
    default:
      Bot_Abort(BOTH_DIR);
      Set_Scsi_Sense_Data(g_sCBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
      Set_Pkt_CSW (CSW_PHASE_ERROR, SEND_CSW_DISABLE);
      break;
  }
	
}
