/******************************************************************************
 * @file     hid_touch.h
 * @brief    LAG020 series USB driver header file
 * @version  2.0.0
 * @date     22, Feb, 2017
 *
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef __USBD_COMPOSITE_H__
#define __USBD_COMPOSITE_H__



#define CTRL_IN_EP		EP0		
#define CTRL_OUT_EP		EP1

#define	EP2_ADDR		(0x02 | EP_INPUT)
#define	EP3_ADDR		(0x03 | EP_INPUT)
#define	EP4_ADDR		(0x04 | EP_INPUT)
#define	EP5_ADDR		(0x05 | EP_INPUT)
#define	EP6_ADDR		(0x06 | EP_INPUT)
#define	EP7_ADDR		(0x07 | EP_INPUT)
#define	EP8_ADDR		(0x08 | EP_INPUT)
#define	EP9_ADDR		(0x09 | EP_INPUT)
#define	EP10_ADDR		(0x0A | EP_INPUT)
#define	EP11_ADDR		(0x0B | EP_INPUT)



/* Define the vendor id and product id */
#define USBD_VID        0x0416    
#define USBD_PID        0x8230    

/*!<Define HID Class Specific Request */
#define GET_REPORT          0x01
#define GET_IDLE            0x02
#define GET_PROTOCOL        0x03
#define SET_REPORT          0x09
#define SET_IDLE            0x0A
#define SET_PROTOCOL        0x0B

/*!<USB HID Interface Class protocol */
#define HID_NONE            0x00
#define HID_KEYBOARD        0x01
#define HID_MOUSE           0x02

/*!<USB HID Class Report Type */
#define HID_RPT_TYPE_INPUT      0x01
#define HID_RPT_TYPE_OUTPUT     0x02
#define HID_RPT_TYPE_FEATURE    0x03

/*-------------------------------------------------------------*/
/* Define EP maximum packet size */
#define EP0_MAX_PKT_SIZE    64		
#define EP1_MAX_PKT_SIZE    EP0_MAX_PKT_SIZE
#define EP2_MAX_PKT_SIZE    64
#define EP3_MAX_PKT_SIZE    64
#define EP4_MAX_PKT_SIZE    64
#define EP5_MAX_PKT_SIZE    64
#define EP6_MAX_PKT_SIZE    64
#define EP7_MAX_PKT_SIZE    64
#define EP8_MAX_PKT_SIZE    64
#define EP9_MAX_PKT_SIZE    64
#define EP10_MAX_PKT_SIZE   64
#define EP11_MAX_PKT_SIZE   64


#define SETUP_BUF_BASE  0
#define SETUP_BUF_LEN   8

#define EP0_BUF_BASE    (SETUP_BUF_BASE + SETUP_BUF_LEN)
#define EP0_BUF_LEN     EP0_MAX_PKT_SIZE
#define EP1_BUF_BASE    (SETUP_BUF_BASE + SETUP_BUF_LEN)
#define EP1_BUF_LEN     EP1_MAX_PKT_SIZE
#define EP2_BUF_BASE    (EP1_BUF_BASE + EP1_BUF_LEN)
#define EP2_BUF_LEN     (EP2_MAX_PKT_SIZE)
#define EP3_BUF_BASE    (EP2_BUF_BASE + EP2_BUF_LEN)
#define EP3_BUF_LEN     (EP3_MAX_PKT_SIZE)
#define EP4_BUF_BASE    (EP3_BUF_BASE + EP3_BUF_LEN)
#define EP4_BUF_LEN     (EP4_MAX_PKT_SIZE)
#define EP5_BUF_BASE    (EP4_BUF_BASE + EP4_BUF_LEN)
#define EP5_BUF_LEN     (EP5_MAX_PKT_SIZE)
#define EP6_BUF_BASE    (EP5_BUF_BASE + EP5_BUF_LEN)
#define EP6_BUF_LEN     (EP6_MAX_PKT_SIZE)
#define EP7_BUF_BASE    (EP6_BUF_BASE + EP6_BUF_LEN)
#define EP7_BUF_LEN     (EP7_MAX_PKT_SIZE)
#define EP8_BUF_BASE    (EP7_BUF_BASE + EP7_BUF_LEN)
#define EP8_BUF_LEN     (EP8_MAX_PKT_SIZE)
#define EP9_BUF_BASE    (EP8_BUF_BASE + EP8_BUF_LEN)
#define EP9_BUF_LEN     (EP9_MAX_PKT_SIZE)
#define EP10_BUF_BASE   (EP9_BUF_BASE + EP9_BUF_LEN)
#define EP10_BUF_LEN    (EP10_MAX_PKT_SIZE)
#define EP11_BUF_BASE   (EP10_BUF_BASE + EP10_BUF_LEN)
#define EP11_BUF_LEN    (EP11_MAX_PKT_SIZE)

/* Define Descriptor information */
#define HID_DEFAULT_INT_IN_INTERVAL     1
#define USBD_SELF_POWERED               0
#define USBD_REMOTE_WAKEUP              0
#define USBD_MAX_POWER                  100  /* The unit is in 2mA. ex: 100 * 2mA = 200mA */

#define USBD_NUM_INTERFACE				1
#define LEN_EACH_INTERFACE      		(LEN_INTERFACE+LEN_HID+LEN_ENDPOINT)
#define LEN_CONFIG_AND_SUBORDINATE      (LEN_CONFIG+LEN_EACH_INTERFACE*USBD_NUM_INTERFACE)

#define IF0_INPUT_RPTID		0x02
#define IF0_OUTPUT_RPTID	0x03
#define IF1_INPUT_RPTID		0x04
#define IF1_OUTPUT_RPTID	0x05
#define IF2_INPUT_RPTID		0x06
#define IF2_OUTPUT_RPTID	0x07
#define IF3_INPUT_RPTID		0x08
#define IF3_OUTPUT_RPTID	0x09
#define IF4_INPUT_RPTID		0x0A
#define IF4_OUTPUT_RPTID	0x0B
#define INPUT_RPTID         IF0_INPUT_RPTID    // backward compatible

/*-------------------------------------------------------------*/
void HID_Init(void);
void HID_ClassRequest(void);
void EP2_Handler(void);
void EP3_Handler(void);
void EP4_Handler(void);
void EP5_Handler(void);
void EP6_Handler(void);
void EP7_Handler(void);
void EP8_Handler(void);
void EP9_Handler(void);
void EP10_Handler(void);
void EP11_Handler(void);
void HID_UpdateTouchData(void);

#endif  /* __USBD_COMPOSITE_H_ */

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
