
#ifndef __USB_BOT_H__
#define __USB_BOT_H__

#include "stdint.h"





/* Mass Storage Memory Layout */    
#define DATA_FLASH_STORAGE_SIZE    (64*1024)         /* Configure the DATA FLASH storage size, To pass USB-IF MSC Test, it needs > 64KB */
#define UDC_SECTOR_SIZE            (4096)   //(512)  /* logic sector size */
#define MSC_BlockCount  (DATA_FLASH_STORAGE_SIZE / UDC_SECTOR_SIZE)
   
#define MASS_STORAGE_OFFSET  0x00010000        /* To avoid the code to write APROM */
#define FLASH_PAGE_SIZE      4096 
#define BUFFER_PAGE_SIZE     512


/* USB Device Classes */
#define USB_DEVICE_CLASS_RESERVED              0x00
#define USB_DEVICE_CLASS_AUDIO                 0x01
#define USB_DEVICE_CLASS_COMMUNICATIONS        0x02
#define USB_DEVICE_CLASS_HUMAN_INTERFACE       0x03
#define USB_DEVICE_CLASS_MONITOR               0x04
#define USB_DEVICE_CLASS_PHYSICAL_INTERFACE    0x05
#define USB_DEVICE_CLASS_POWER                 0x06
#define USB_DEVICE_CLASS_PRINTER               0x07
#define USB_DEVICE_CLASS_STORAGE               0x08
#define USB_DEVICE_CLASS_HUB                   0x09
#define USB_DEVICE_CLASS_MISCELLANEOUS         0xEF
#define USB_DEVICE_CLASS_VENDOR_SPECIFIC       0xFF


/* MSC Subclass Codes */
#define MSC_SUBCLASS_RBC                0x01
#define MSC_SUBCLASS_SFF8020I_MMC2      0x02
#define MSC_SUBCLASS_QIC157             0x03
#define MSC_SUBCLASS_UFI                0x04
#define MSC_SUBCLASS_SFF8070I           0x05
#define MSC_SUBCLASS_SCSI               0x06		// Small Computer System Interface 

/* MSC Protocol Codes */
#define MSC_PROTOCOL_CBI_INT            0x00
#define MSC_PROTOCOL_CBI_NOINT          0x01
#define MSC_PROTOCOL_BULK_ONLY          0x50		// BULK-ONLY TRANSPORT 


/*!<Define Mass Storage Class Specific Request */
#define BULK_ONLY_MASS_STORAGE_RESET    0xFF
#define GET_MAX_LUN                     0xFE


/* MSC Bulk-only Stage */
#define BOT_IDLE                      0       /* Idle state */
#define BOT_DATA_OUT                  1       /* Data Out state */
#define BOT_DATA_IN                   2       /* Data In state */
#define BOT_DATA_IN_LAST              3       /* Last Data In Last */
#define BOT_CSW_Send                  4       /* Command Status Wrapper */
#define BOT_ERROR                     5       /* error state */
#define BOT_DATA_IN_LAST_STALL        6       /* Last Data In Last with Stall*/


#define BOT_CBW_SIGNATURE             0x43425355
#define BOT_CSW_SIGNATURE             0x53425355
#define BOT_CBW_PACKET_LENGTH         31
#define CSW_DATA_LENGTH               13

/* CSW Status Definitions */
#define CSW_CMD_PASSED                0x00
#define CSW_CMD_FAILED                0x01
#define CSW_PHASE_ERROR               0x02


#define DIR_IN                        0
#define DIR_OUT                       1
#define BOTH_DIR                      2

#define SEND_CSW_DISABLE              0
#define SEND_CSW_ENABLE               1

#define MAX_LUN  1


/* Bulk-only Command Block Wrapper */
typedef struct _Bulk_Only_CBW
{
  uint32_t dSignature;
  uint32_t dTag;
  uint32_t dDataLength;
  uint8_t  bmFlags;
  uint8_t  bLUN;
  uint8_t  bCBLength;
  uint8_t  CB[16];
}
Bulk_Only_CBW;	 

/* Bulk-only Command Status Wrapper */
typedef struct _Bulk_Only_CSW
{
  uint32_t dSignature;
  uint32_t dTag;
  uint32_t dDataResidue;
  uint8_t  bStatus;
}
Bulk_Only_CSW;


extern Bulk_Only_CBW 	g_sCBW;                  /* Command Block Wrapper */
extern Bulk_Only_CSW 	g_sCSW;                  /* Command Status Wrapper */

extern uint8_t   g_u8BotState;
extern uint8_t   g_au8Memory[];  								/* MSC RAM */
extern uint32_t  g_u32MemOK;                   	/* Memory OK flag */
extern uint32_t  g_u32Offset;                  	/* Memory R/W Address */
extern uint32_t  g_u32Length;                  	/* Memory R/W Length */
extern uint8_t   g_u8BulkLen;                 	/* Bulk In/Out Length */
extern uint8_t   g_au8BulkBuf[]; 								/* Bulk In/Out Buffer */
extern uint32_t  g_u32DataFlashStartAddr;



/* MSC Bulk Callback Functions */
void Mass_Storage_In (void);
void Mass_Storage_Out (void);
void CBW_Pkt_Decode(void);
void Transfer_Data_Request(uint8_t* Data_Pointer, uint16_t Data_Len, uint8_t Enable_Stall);
void Set_Pkt_CSW (uint8_t CSW_Status, uint8_t Send_Permission);
void Bot_Abort(uint8_t Direction);
uint32_t MSC_Reset (void);


typedef struct _SCSI_COMMAND_REQUESTS
{
	void (*TestUnitReady_Cmnd)(uint8_t);
	void (*RequestSense_Cmnd)(uint8_t);
	void (*Inquiry_Cmnd)(uint8_t);
	void (*Start_Stop_Unit_Cmnd)(uint8_t);
	void (*ModeSense6_Cmnd)(uint8_t);
	void (*ModeSense10_Cmnd)(uint8_t);
	void (*ReadFormatCapacity_Cmnd)(uint8_t);
	void (*ReadCapacity10_Cmnd)(uint8_t);
	void (*ReadCapacity16_Cmnd)(uint8_t);
	void (*Write10_Cmnd)(uint8_t);
	void (*Read10_Cmnd)(uint8_t);
	void (*Verify10_Cmnd)(uint8_t);
	void (*Format_Cmnd)(uint8_t);
	//////////////////////////////////////////////////////////
	/*Unsupported command*/
	void (*Send_Diagnostic_Cmnd)(uint8_t);
	void (*Mode_Select10_Cmnd)(uint8_t);
	void (*Mode_Select6_Cmnd)(uint8_t);
	void (*Read6_Cmnd)(uint8_t);
	void (*Read16_Cmnd)(uint8_t);
	void (*Write6_Cmnd)(uint8_t);
	void (*Write16_Cmnd)(uint8_t);
	void (*Verify12_Cmnd)(uint8_t);
	void (*Verify16_Cmnd)(uint8_t);
} 
SCSI_COMMAND_REQUESTS;


#endif  /* __USB_BOT_H__ */
