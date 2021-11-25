/******************************************************************************
 * @file     usbd_scsi.c
 * @brief    
 *
 * @note
 * Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "massstorage.h"
#include "usbd_bot.h"
#include "usbd_scsi.h"
#include "dataflash.h"


uint8_t g_au8Page00_Inquiry_Data[] =
{
	0x00, /* PERIPHERAL QUALIFIER & PERIPHERAL DEVICE TYPE*/
	0x00,
	0x00,
	0x00,
	0x00 /* Supported Pages 00*/
};

uint8_t g_au8Standard_Inquiry_Data[] =
{
	0x00,          /* Direct Access Device */
	0x80,          /* RMB = 1: Removable Medium */
	0x02,          /* Version: No conformance claim to standard */
	0x02,

	36 - 4,          /* Additional Length */
	0x00,          /* SCCS = 1: Storage Controller Component */
	0x00,
	0x00,
	/* Vendor Identification */
	'N', 'u', 'v', 'o', 't', 'o', 'n', ' ',
	/* Product Identification */
	'U', 'S', 'B', ' ', 'F', 'l', 'a', 's', 'h',
	'D', 'i', 's', 'k', ' ', ' ',
	/* Product Revision Level */
	'1', '.', '0', ' '
};

uint8_t g_au8Standard_Inquiry_Data2[] =
{
	0x00,          /* Direct Access Device */
	0x80,          /* RMB = 1: Removable Medium */
	0x02,          /* Version: No conformance claim to standard */
	0x02,

	36 - 4,        /* Additional Length */
	0x00,          /* SCCS = 1: Storage Controller Component */
	0x00,
	0x00,
	/* Vendor Identification */
	'N', 'u', 'v', 'o', 't', 'o', 'n', ' ',
	/* Product Identification */
	'N', 'A', 'N', 'D', ' ', 'F', 'l', 'a', 's', 'h', ' ',
	'D', 'i', 's', 'k', ' ',
	/* Product Revision Level */
	'1', '.', '0', ' '
};
	

uint8_t g_au8Mode_Sense6_data[] =
{
	0x03,
	0x00,
	0x00,
	0x00,
};


uint8_t g_au8Mode_Sense10_data[] =
{
	0x00,
	0x06,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00
};

uint8_t g_au8Scsi_Sense_Data[] =
{
	0x70, /*RespCode*/
	0x00, /*SegmentNumber*/
	NO_SENSE, /* Sens_Key*/
	0x00,
	0x00,
	0x00,
	0x00, /*Information*/
	0x0A, /*AdditionalSenseLength*/
	0x00,
	0x00,
	0x00,
	0x00, /*CmdInformation*/
	NO_SENSE, /*Asc*/
	0x00, /*ASCQ*/
	0x00, /*FRUC*/
	0x00, /*TBD*/
	0x00,
	0x00 /*SenseKeySpecific*/
};
	
uint8_t g_au8ReadCapacity10_Data[] =
{
	/* Last Logical Block */
	0,
	0,
	0,
	0,

	/* Block Length */
	0,
	0,
	0,
	0
};

uint8_t g_au8ReadFormatCapacity_Data[] =
{
	0x00,
	0x00,
	0x00,
	0x08, /* Capacity List Length */

	/* Block Count */
	0,
	0,
	0,
	0,

	/* Block Length */
	0x02,/* Descriptor Code: Formatted Media */
	0,
	0,
	0
};




/*******************************************************************************
* Function Name  : SCSI_Inquiry_Cmnd
* Description    : SCSI Inquiry Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_Inquiry_Cmnd(uint8_t lun)
{
  uint8_t* Inquiry_Data;
  uint16_t Inquiry_Data_Length;
	uint8_t  Stall_Flag;

  if (g_sCBW.CB[1] & 0x01)  /*Evpd is set*/
  {
    Inquiry_Data = g_au8Page00_Inquiry_Data;
    Inquiry_Data_Length = 42;
  }
  else
  {

    if ( lun == 0)
    {
      Inquiry_Data = g_au8Standard_Inquiry_Data;
    }
    else
    {
      Inquiry_Data = g_au8Standard_Inquiry_Data2;
    }

    if (g_sCBW.CB[4] <= STANDARD_INQUIRY_DATA_LEN)
		{
			Stall_Flag = 0;
      Inquiry_Data_Length = g_sCBW.CB[4];
		}
    else
		{
			Stall_Flag = 1;
      Inquiry_Data_Length = STANDARD_INQUIRY_DATA_LEN;
		}
		
  }
	
	Transfer_Data_Request(Inquiry_Data, Inquiry_Data_Length, Stall_Flag);
		
}

/*******************************************************************************
* Function Name  : SCSI_ReadFormatCapacity_Cmnd
* Description    : SCSI ReadFormatCapacity Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_ReadFormatCapacity_Cmnd (uint8_t lun)
{
	if ( lun != 0 ) 
  {
    Set_Scsi_Sense_Data(g_sCBW.bLUN, NOT_READY, MEDIUM_NOT_PRESENT);
    Set_Pkt_CSW (CSW_CMD_FAILED, SEND_CSW_ENABLE);
    Bot_Abort(DIR_IN);
    return;
  }
	
  g_au8ReadFormatCapacity_Data[4] = (uint8_t)(MSC_BlockCount >> 24);
  g_au8ReadFormatCapacity_Data[5] = (uint8_t)(MSC_BlockCount >> 16);
  g_au8ReadFormatCapacity_Data[6] = (uint8_t)(MSC_BlockCount >>  8);
  g_au8ReadFormatCapacity_Data[7] = (uint8_t)(MSC_BlockCount);

	g_au8ReadFormatCapacity_Data[8] = 0x02;
  g_au8ReadFormatCapacity_Data[9] = (uint8_t)(UDC_SECTOR_SIZE >>  16);
  g_au8ReadFormatCapacity_Data[10] = (uint8_t)(UDC_SECTOR_SIZE >>  8);
  g_au8ReadFormatCapacity_Data[11] = (uint8_t)(UDC_SECTOR_SIZE);
	
  Transfer_Data_Request(g_au8ReadFormatCapacity_Data, READ_FORMAT_CAPACITY_DATA_LEN, 0);
}

/*******************************************************************************
* Function Name  : SCSI_ReadCapacity10_Cmnd
* Description    : SCSI ReadCapacity10 Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_ReadCapacity10_Cmnd(uint8_t lun)
{

	if ( lun != 0 ) 
  {
    Set_Scsi_Sense_Data(g_sCBW.bLUN, NOT_READY, MEDIUM_NOT_PRESENT);
    Set_Pkt_CSW (CSW_CMD_FAILED, SEND_CSW_ENABLE);
    Bot_Abort(DIR_IN);
    return;
  }

  g_au8ReadCapacity10_Data[0] = (uint8_t)((MSC_BlockCount - 1) >> 24);
  g_au8ReadCapacity10_Data[1] = (uint8_t)((MSC_BlockCount - 1) >> 16);
  g_au8ReadCapacity10_Data[2] = (uint8_t)((MSC_BlockCount - 1) >>  8);
  g_au8ReadCapacity10_Data[3] = (uint8_t)(MSC_BlockCount - 1);

  g_au8ReadCapacity10_Data[4] = (uint8_t)(UDC_SECTOR_SIZE >>  24);
  g_au8ReadCapacity10_Data[5] = (uint8_t)(UDC_SECTOR_SIZE >>  16);
  g_au8ReadCapacity10_Data[6] = (uint8_t)(UDC_SECTOR_SIZE >>  8);
  g_au8ReadCapacity10_Data[7] = (uint8_t)(UDC_SECTOR_SIZE);
	
  Transfer_Data_Request(g_au8ReadCapacity10_Data, READ_CAPACITY10_DATA_LEN, 0);
}

/*******************************************************************************
* Function Name  : SCSI_ReadCapacity16_Cmnd
* Description    : SCSI ReadCapacity16 Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_ReadCapacity16_Cmnd (uint8_t lun)
{	
	uint8_t Len, Buff[32];
	uint8_t Stall_Flag;

	memset(Buff, 0, 32);
		
	if ( lun != 0 ) 
  {
    Set_Scsi_Sense_Data(g_sCBW.bLUN, NOT_READY, MEDIUM_NOT_PRESENT);
    Set_Pkt_CSW (CSW_CMD_FAILED, SEND_CSW_ENABLE);
    Bot_Abort(DIR_IN);
    return;
  }
	
	if (g_sCBW.dDataLength <= 32)	
	{
		Stall_Flag = 0; 
		Len = g_sCBW.dDataLength;
	}
	else
	{
		Stall_Flag = 1; 
		Len = 32;
	}
	
	Buff[ 0] = 0x0;
	Buff[ 1] = 0x0;
	Buff[ 2] = 0x0;
	Buff[ 3] = 0x0;
	Buff[ 4] = (uint8_t)((MSC_BlockCount - 1) >> 24);
	Buff[ 5] = (uint8_t)((MSC_BlockCount - 1) >> 16);
	Buff[ 6] = (uint8_t)((MSC_BlockCount - 1) >>  8);
	Buff[ 7] = (uint8_t)(MSC_BlockCount - 1);
	Buff[ 8] = (uint8_t)(UDC_SECTOR_SIZE >>  24);
  Buff[ 9] = (uint8_t)(UDC_SECTOR_SIZE >>  16);
  Buff[10] = (uint8_t)(UDC_SECTOR_SIZE >>  8);
  Buff[11] = (uint8_t)(UDC_SECTOR_SIZE);
	
	Transfer_Data_Request(Buff, Len, Stall_Flag);
}

/*******************************************************************************
* Function Name  : SCSI_ModeSense6_Cmnd
* Description    : SCSI ModeSense6 Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_ModeSense6_Cmnd (uint8_t lun)
{
  Transfer_Data_Request(g_au8Mode_Sense6_data, MODE_SENSE6_DATA_LEN, 0);
}

/*******************************************************************************
* Function Name  : SCSI_ModeSense10_Cmnd
* Description    : SCSI ModeSense10 Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_ModeSense10_Cmnd (uint8_t lun)
{
  Transfer_Data_Request(g_au8Mode_Sense10_data, MODE_SENSE10_DATA_LEN, 0);
}

/*******************************************************************************
* Function Name  : SCSI_RequestSense_Cmnd
* Description    : SCSI RequestSense Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_RequestSense_Cmnd (uint8_t lun)
{
  uint8_t Request_Sense_data_Length;
	uint8_t Stall_Flag;

  if (g_sCBW.CB[4] <= REQUEST_SENSE_DATA_LEN)
  {
		Stall_Flag = 0;
    Request_Sense_data_Length = g_sCBW.CB[4];
  }
  else
  {
		Stall_Flag = 1;
    Request_Sense_data_Length = REQUEST_SENSE_DATA_LEN;
  }
	
  Transfer_Data_Request(g_au8Scsi_Sense_Data, Request_Sense_data_Length, Stall_Flag);
}

/*******************************************************************************
* Function Name  : Set_Scsi_Sense_Data
* Description    : Set Scsi Sense Data routine.
* Input          : uint8_t Sens_Key
                   uint8_t Asc.
* Output         : None.
* Return         : None.
*******************************************************************************/
void Set_Scsi_Sense_Data(uint8_t lun, uint8_t Sens_Key, uint8_t Asc)
{
  g_au8Scsi_Sense_Data[ 2] = Sens_Key;
  g_au8Scsi_Sense_Data[12] = Asc;
}

/*******************************************************************************
* Function Name  : SCSI_Start_Stop_Unit_Cmnd
* Description    : SCSI Start_Stop_Unit Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_Start_Stop_Unit_Cmnd(uint8_t lun)
{
  Set_Pkt_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
}


/*******************************************************************************
* Function Name  : SCSI_Read10_Cmnd
* Description    : SCSI Read10 Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
* Error_1        : CBW.dDataLength = 0
* Error_2        : CBW.dDataLength != g_u32Length
* Error_3        : address is out of range
* Error_4        : CBW.bmFlags bit7 = 0  
*******************************************************************************/
void SCSI_Read10_Cmnd(uint8_t lun)
{
  uint32_t Transmit_Len;
	
  if (g_u8BotState == BOT_IDLE)
  {
		 Transmit_Len = GetMin(g_u32Length, MSC_MAX_PACKET); 
		 if (g_sCBW.dDataLength == 0 )   
		 {
			  Set_Scsi_Sense_Data(lun, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
        Set_Pkt_CSW (CSW_CMD_FAILED, SEND_CSW_ENABLE);
			  return;
		 }
		 
		 if (g_sCBW.dDataLength != g_u32Length )
	   { 
			  Bot_Abort(DIR_IN);
			  Set_Scsi_Sense_Data(lun, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
        Set_Pkt_CSW (CSW_CMD_FAILED, SEND_CSW_ENABLE);
			  return;
	   }
		 
		 
		 if ((g_u32Offset + Transmit_Len) > DATA_FLASH_STORAGE_SIZE) 		
		 {		
			  Bot_Abort(DIR_IN);
			  Set_Scsi_Sense_Data(lun, ILLEGAL_REQUEST, ADDRESS_OUT_OF_RANGE);
			  Set_Pkt_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);		
			  return;
		 }
		
		 if ((g_sCBW.bmFlags & 0x80) != 0)
		 {
				g_u8BotState = BOT_DATA_IN;		
				MSC_MemoryRead();		
		 }
		 else
		 {
				Bot_Abort(BOTH_DIR);
				Set_Scsi_Sense_Data(lun, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
				Set_Pkt_CSW (CSW_CMD_FAILED, SEND_CSW_ENABLE);
		 }
	}
	else if (g_u8BotState == BOT_DATA_IN)
  {
			MSC_MemoryRead();			
  }	 
   
}



/*******************************************************************************
* Function Name  : SCSI_Write10_Cmnd
* Description    : SCSI Write10 Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
* Error_1        : CBW.dDataLength = 0
* Error_2        : CBW.dDataLength != g_u32Length
* Error_3        : address is out of range
* Error_4        : CBW.bmFlags bit7 = 1 
*******************************************************************************/
void SCSI_Write10_Cmnd(uint8_t lun)
{
	if (g_u8BotState == BOT_IDLE)		
  {
		 
		 if (g_sCBW.dDataLength == 0 )				
		 {
			  Set_Scsi_Sense_Data(lun, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
        Set_Pkt_CSW (CSW_CMD_FAILED, SEND_CSW_ENABLE);
			  return;
		 }
		 
		 if (g_sCBW.dDataLength != g_u32Length )	
	   { 
			  Bot_Abort(BOTH_DIR);
		    Set_Scsi_Sense_Data(lun, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
        Set_Pkt_CSW (CSW_CMD_FAILED, SEND_CSW_ENABLE);
		    return;
	   }
		 
		 if ((g_u32Offset + g_u8BulkLen) > DATA_FLASH_STORAGE_SIZE) 
		 {
				 g_u8BulkLen = DATA_FLASH_STORAGE_SIZE - g_u32Offset;
				 g_u8BotState = BOT_CSW_Send;		
				
				 Bot_Abort(BOTH_DIR);
				 Set_Scsi_Sense_Data(lun, ILLEGAL_REQUEST, ADDRESS_OUT_OF_RANGE);
				 Set_Pkt_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);		
				 return;
		 }
		 
		 if ((g_sCBW.bmFlags & 0x80) == 0)
		 {
				 g_u8BotState = BOT_DATA_OUT;					
				 
				 USBD_SET_PAYLOAD_LEN(BULK_OUT_EP, EP3_MAX_PKT_SIZE);    
		 }
		 else
		 {
				 Bot_Abort(DIR_IN);
				 Set_Scsi_Sense_Data(g_sCBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
				 //Set_Pkt_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
				 Set_Pkt_CSW (CSW_CMD_FAILED, SEND_CSW_ENABLE);
		 }
		 return;
  }
  else if (g_u8BotState == BOT_DATA_OUT)
  {
		 MSC_MemoryWrite();		 	
  }
	
}

/*******************************************************************************
* Function Name  : SCSI_Verify10_Cmnd
* Description    : SCSI Verify10 Command routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_Verify10_Cmnd(uint8_t lun)
{
  if ((g_sCBW.dDataLength == 0) && !(g_sCBW.CB[1] & BLKVFY))/* BLKVFY not set*/
  {
    Set_Pkt_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
  }
  else
  {
    Bot_Abort(BOTH_DIR);
    Set_Scsi_Sense_Data(g_sCBW.bLUN, ILLEGAL_REQUEST, INVALID_FIELED_IN_COMMAND);
    Set_Pkt_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
  }
}
/*******************************************************************************
* Function Name  : SCSI_Valid_Cmnd
* Description    : Valid Commands routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_Valid_Cmnd(uint8_t lun)
{
  if (g_sCBW.dDataLength != 0)
  {
    Bot_Abort(BOTH_DIR);
    Set_Scsi_Sense_Data(g_sCBW.bLUN, ILLEGAL_REQUEST, INVALID_COMMAND);
    Set_Pkt_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
  }
  else
    Set_Pkt_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);
}
/*******************************************************************************
* Function Name  : SCSI_Valid_Cmnd
* Description    : Valid Commands routine.
* Input          : None.
* Output         : None.
* Return         : None.
* Error          : Invalid CBW.dDataLength
*******************************************************************************/
void SCSI_TestUnitReady_Cmnd(uint8_t lun)
{
	
	if ( g_sCBW.dDataLength  != 0 ) 
	{
		 if ( g_sCBW.bmFlags & 0x80 ) 
		 {
		 		MSC_SetStallEP(BULK_IN_EP);
		 }	
		 else
		 { 
			 MSC_SetStallEP(BULK_OUT_EP);
		 }		
		 Set_Pkt_CSW (CSW_PHASE_ERROR, SEND_CSW_ENABLE);	
	}
	
	if ( lun != 0 ) 
  {
     Set_Scsi_Sense_Data(g_sCBW.bLUN, NOT_READY, MEDIUM_NOT_PRESENT);
     Set_Pkt_CSW (CSW_CMD_FAILED, SEND_CSW_ENABLE);
     Bot_Abort(DIR_IN);
     return;
  }
  else
  {		
     Set_Pkt_CSW (CSW_CMD_PASSED, SEND_CSW_ENABLE);		
  }
}
/*******************************************************************************
* Function Name  : SCSI_Format_Cmnd
* Description    : Format Commands routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_Format_Cmnd(uint8_t lun)
{
  if ( lun != 0 ) 
  {
    Set_Scsi_Sense_Data(g_sCBW.bLUN, NOT_READY, MEDIUM_NOT_PRESENT);
    Set_Pkt_CSW (CSW_CMD_FAILED, SEND_CSW_ENABLE);
    Bot_Abort(DIR_IN);
    return;
  }
}
/*******************************************************************************
* Function Name  : SCSI_Invalid_Cmnd
* Description    : Invalid Commands routine
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SCSI_Invalid_Cmnd(uint8_t lun)
{
  if (g_sCBW.dDataLength == 0)
  {
    Bot_Abort(DIR_IN);
  }
  else
  {
    if ((g_sCBW.bmFlags & 0x80) != 0)
    {
      Bot_Abort(DIR_IN);
    }
    else
    {
      Bot_Abort(BOTH_DIR);
    }
  }
	
  Set_Scsi_Sense_Data(g_sCBW.bLUN, ILLEGAL_REQUEST, INVALID_COMMAND);
  Set_Pkt_CSW (CSW_CMD_FAILED, SEND_CSW_DISABLE);
}

