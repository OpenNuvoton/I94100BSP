

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_SCSI_H
#define __USB_SCSI_H


/* Includes ------------------------------------------------------------------*/
#include "stdint.h"
//#include "usb_type.h"


/* SCSI Commands */
#define SCSI_FORMAT_UNIT               0x04
#define SCSI_INQUIRY                   0x12
#define SCSI_MODE_SELECT6              0x15
#define SCSI_MODE_SELECT10             0x55
#define SCSI_MODE_SENSE6               0x1A
#define SCSI_MODE_SENSE10              0x5A
#define SCSI_ALLOW_MEDIUM_REMOVAL      0x1E
#define SCSI_READ6                     0x08
#define SCSI_READ10                    0x28
#define SCSI_READ12                    0xA8		
#define SCSI_READ16                    0x88
#define SCSI_READ_CAPACITY10           0x25
#define SCSI_READ_CAPACITY16           0x9E		
#define SCSI_REQUEST_SENSE             0x03
#define SCSI_START_STOP_UNIT           0x1B
#define SCSI_TEST_UNIT_READY           0x00
#define SCSI_WRITE6                    0x0A
#define SCSI_WRITE10                   0x2A
#define SCSI_WRITE12                   0xAA		
#define SCSI_WRITE16                   0x8A
#define SCSI_VERIFY10                  0x2F
#define SCSI_VERIFY12                  0xAF
#define SCSI_VERIFY16                  0x8F
#define SCSI_SEND_DIAGNOSTIC           0x1D
#define SCSI_READ_FORMAT_CAPACITIES    0x23

#define NO_SENSE		                   0
#define RECOVERED_ERROR		             1
#define NOT_READY		                   2
#define MEDIUM_ERROR		               3
#define HARDWARE_ERROR		             4
#define ILLEGAL_REQUEST		             5
#define UNIT_ATTENTION		             6
#define DATA_PROTECT		               7
#define BLANK_CHECK		                 8
#define VENDOR_SPECIFIC		             9
#define COPY_ABORTED		               10
#define ABORTED_COMMAND		             11
#define VOLUME_OVERFLOW		             13
#define MISCOMPARE		                 14


#define INVALID_COMMAND                             0x20
#define INVALID_FIELED_IN_COMMAND                   0x24
#define PARAMETER_LIST_LENGTH_ERROR                 0x1A
#define INVALID_FIELD_IN_PARAMETER_LIST             0x26
#define ADDRESS_OUT_OF_RANGE                        0x21
#define MEDIUM_NOT_PRESENT 			    								0x3A
#define MEDIUM_HAVE_CHANGED			    								0x28

#define READ_FORMAT_CAPACITY_DATA_LEN               0x0C
#define READ_CAPACITY10_DATA_LEN                    0x08
#define MODE_SENSE10_DATA_LEN                       0x08
#define MODE_SENSE6_DATA_LEN                        0x04
#define REQUEST_SENSE_DATA_LEN                      0x12
#define STANDARD_INQUIRY_DATA_LEN                   0x24
#define BLKVFY                                      0x04


/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void SCSI_Inquiry_Cmnd(uint8_t lun);
void SCSI_ReadFormatCapacity_Cmnd(uint8_t lun);
void SCSI_ReadCapacity10_Cmnd(uint8_t lun);
void SCSI_ReadCapacity16_Cmnd(uint8_t lun);
void SCSI_RequestSense_Cmnd(uint8_t lun);
void SCSI_Start_Stop_Unit_Cmnd(uint8_t lun);
void SCSI_ModeSense6_Cmnd(uint8_t lun);
void SCSI_ModeSense10_Cmnd (uint8_t lun);
void SCSI_Write10_Cmnd(uint8_t lun);
void SCSI_Read10_Cmnd(uint8_t lun);
void SCSI_Verify10_Cmnd(uint8_t lun);
void SCSI_Invalid_Cmnd(uint8_t lun);
void SCSI_Valid_Cmnd(uint8_t lun);
void Set_Scsi_Sense_Data(uint8_t lun , uint8_t Sens_Key, uint8_t Asc);
void SCSI_TestUnitReady_Cmnd (uint8_t lun);
void SCSI_Format_Cmnd (uint8_t lun);


/* Invalid (Unsupported) commands */
#define SCSI_Write6_Cmnd                  SCSI_Invalid_Cmnd
#define SCSI_Write16_Cmnd                 SCSI_Invalid_Cmnd
#define SCSI_Read6_Cmnd                   SCSI_Invalid_Cmnd
#define SCSI_Read16_Cmnd                  SCSI_Invalid_Cmnd
#define SCSI_Mode_Select6_Cmnd            SCSI_Invalid_Cmnd
#define SCSI_Mode_Select10_Cmnd           SCSI_Invalid_Cmnd
#define SCSI_Send_Diagnostic_Cmnd         SCSI_Invalid_Cmnd
#define SCSI_Verify12_Cmnd                SCSI_Invalid_Cmnd
#define SCSI_Verify16_Cmnd                SCSI_Invalid_Cmnd

#endif /* __USB_SCSI_H */



