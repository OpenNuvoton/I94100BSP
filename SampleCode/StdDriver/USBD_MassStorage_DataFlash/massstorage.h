/******************************************************************************
 * @file     massstorage.h
 * @brief    LAG020 series USB mass storage header file
 *
 * @note
 * Copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#include "stdint.h" 
 

/*Get MAX or MIN value*/
#define GetMax( x1, x2 ) ( ( x1 ) > ( x2 ) ? ( x1 ) : ( x2 ) )
#define GetMin( x1, x2 ) ( ( x1 ) > ( x2 ) ? ( x2 ) : ( x1 ) )

#define  LOW_BYTE(x)		( x & 0xFF ) 
#define  HIGH_BYTE(x) 	((x >> 8) & 0xFF ) 


/* Define the vendor id and product id */
#define USBD_VID        0x0416
#define USBD_PID        0x1100

/* Define EP maximum packet size */
#define EP0_MAX_PKT_SIZE    64
#define EP1_MAX_PKT_SIZE    EP0_MAX_PKT_SIZE
#define EP2_MAX_PKT_SIZE    64
#define EP3_MAX_PKT_SIZE    64

#define SETUP_BUF_BASE      0
#define SETUP_BUF_LEN       8
#define EP0_BUF_BASE        (SETUP_BUF_BASE + SETUP_BUF_LEN)
#define EP0_BUF_LEN         (EP0_MAX_PKT_SIZE)
#define EP1_BUF_BASE        (SETUP_BUF_BASE + SETUP_BUF_LEN)
#define EP1_BUF_LEN         (EP1_MAX_PKT_SIZE)
#define EP2_BUF_BASE        (EP1_BUF_BASE + EP1_BUF_LEN)
#define EP2_BUF_LEN         (EP2_MAX_PKT_SIZE)
#define EP3_BUF_BASE        (EP2_BUF_BASE + EP2_BUF_LEN)
#define EP3_BUF_LEN         (EP3_MAX_PKT_SIZE)

/* Define the interrupt In EP number */
#define BULK_IN_EP      0x02
#define BULK_OUT_EP     0x03


/* Define Descriptor information */
#define USBD_SELF_POWERED               0
#define USBD_REMOTE_WAKEUP              0
#define USBD_MAX_POWER                  100  /* The unit is in 2mA. ex: 100 * 2mA = 200mA */

#define LEN_CONFIG_AND_SUBORDINATE      (LEN_CONFIG+LEN_INTERFACE+LEN_ENDPOINT*2)


/* Max In/Out Packet Size */
#define MSC_MAX_PACKET  64


/******************************************************************************/
/*                USBD Mass Storage Structure                                 */
/******************************************************************************/
/** @addtogroup LAG020_USBD_Mass_Exported_Struct M480 USBD Mass Exported Struct
  LAG020 USBD Mass Specific Struct
  @{
*/


static __INLINE uint32_t get_be32(uint8_t *buf)
{
    return ((uint32_t) buf[0] << 24) | ((uint32_t) buf[1] << 16) |
           ((uint32_t) buf[2] << 8) | ((uint32_t) buf[3]);
}


/*-------------------------------------------------------------*/

typedef __packed union 
{
	__packed struct
	{
		uint8_t Recipient : 5;
		uint8_t Type      : 2;    // Bit[6:5] 
		uint8_t Dir       : 1;		// Bit7
	} BM;
	
	uint8_t B;	
} REQUEST_TYPE;
	
typedef __packed union 
{
	__packed struct 
	{
		uint8_t L;
		uint8_t H;
	} WB;
	
	uint16_t W;	
} DB16;

typedef __packed struct 
{
  //uint8_t     bmRequestType;
	REQUEST_TYPE 	bmRequestType;
	uint8_t   		bRequest;
	DB16  				wValue;
	DB16  				wIndex;
	DB16  				wLength;
} SetupPkt_t;




/*-------------------------------------------------------------*/
void MSC_Init(void);
void MSC_Write(void);
void MSC_ClassRequest(void);
void MSC_SetConfig(void);
BOOL MSC_IsSuspended(void);

/*-------------------------------------------------------------*/
void EP2_Handler(void);
void EP3_Handler(void);


/*** (C) COPYRIGHT 2016 Nuvoton Technology Corp. ***/
