/******************************************************************************
 * @file     MassStorage.c
 * @brief    LAG020 series USBD driver Sample file
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
#include "fmc.h"

#if 0
#define DBG_PRINTF      printf
#else
#define DBG_PRINTF(...)
#endif

/*--------------------------------------------------------------------------*/
/* Global variables for Control Pipe */
int32_t g_TotalSectors = 0;
uint32_t g_u32BulkBuf0, g_u32BulkBuf1;

/*--------------------------------------------------------------------------*/
uint8_t g_au8InquiryID[36] = {
    0x00,                   /* Peripheral Device Type */
    0x80,                   /* RMB */
    0x00,                   /* ISO/ECMA, ANSI Version */
    0x00,                   /* Response Data Format */
    0x1F, 0x00, 0x00, 0x00, /* Additional Length */

    /* Vendor Identification */
    'N', 'u', 'v', 'o', 't', 'o', 'n', ' ',

    /* Product Identification */
    'U', 'S', 'B', ' ', 'M', 'a', 's', 's', ' ', 'S', 't', 'o', 'r', 'a', 'g', 'e',

    /* Product Revision */
    '1', '.', '0', '0'
};


void USBD_IRQHandler(void)
{
    uint32_t u32IntSts = USBD_GET_INT_FLAG();
    uint32_t u32State = USBD_GET_BUS_STATE();

    if (u32IntSts & USBD_INTSTS_FLDET) {
        // Floating detect
        USBD_CLR_INT_FLAG(USBD_INTSTS_FLDET);

        if (USBD_IS_ATTACHED())  
				{
			// Enable GPB15(VBUS) pull down state to solute suspend event issue.
			GPIO_EnablePullState(PB,BIT15,GPIO_PUSEL_PULL_DOWN); 	
			
			USBD_CLR_SE0();		
            /* USB Plug In */
            USBD_ENABLE_USB();
			DBG_PRINTF("Plug-In");
        } 
		else 
		{		
			// Disable GPB15 pull down state.
			GPIO_DisablePullState(PB,BIT15); 
            /* USB Un-plug */
            USBD_DISABLE_USB();
			DBG_PRINTF("Un-plug");
			USBD_SET_SE0();		
        }
    }

    if (u32IntSts & USBD_INTSTS_BUS) {
        /* Clear event flag */
        USBD_CLR_INT_FLAG(USBD_INTSTS_BUS);

        if(u32State & USBD_STATE_USBRST) {
            /* Bus reset */
            USBD_ENABLE_USB();
            USBD_SwReset();
            DBG_PRINTF("Bus reset\n");
        }
        if(u32State & USBD_STATE_SUSPEND) {
            /* Enable USB but disable PHY */
            USBD_DISABLE_PHY();
            DBG_PRINTF("Suspend\n");			
        }
        if(u32State & USBD_STATE_RESUME) {
            /* Enable USB and enable PHY */
            USBD_ENABLE_USB();
            DBG_PRINTF("Resume\n");
        }
    }

    if (u32IntSts & USBD_INTSTS_USB) {
        // EP events
        if(u32IntSts & USBD_INTSTS_EP0) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP0);
            // control IN
            USBD_CtrlIn();
        }

        if(u32IntSts & USBD_INTSTS_EP1) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP1);
            // control OUT
            USBD_CtrlOut();
        }

        if(u32IntSts & USBD_INTSTS_EP2) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP2);
            // Bulk IN (MSC)
            EP2_Handler();
        }

        if(u32IntSts & USBD_INTSTS_EP3) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP3);
            // Bulk OUT (MSC)
            EP3_Handler();
        }

        if(u32IntSts & USBD_INTSTS_EP4) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP4);
        }

        if(u32IntSts & USBD_INTSTS_EP5) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP5);
        }

        if(u32IntSts & USBD_INTSTS_EP6) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP6);
        }

        if(u32IntSts & USBD_INTSTS_EP7) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP7);
        }

        // USB event
        if(u32IntSts & USBD_INTSTS_SETUP) {
            // Setup packet
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_SETUP);

            /* Clear the data IN/OUT ready flag of control end-points */
            USBD_STOP_TRANSACTION(EP0);
            USBD_STOP_TRANSACTION(EP1);

            USBD_ProcessSetupPacket();
        }

    }

}


void EP2_Handler(void)
{
		/* Bulk IN */
		Mass_Storage_In();
}


void EP3_Handler(void)
{
    /* Bulk OUT */
	  Mass_Storage_Out();
}



void MSC_Init(void)
{
    int32_t i;
    uint8_t *pu8;

    /* Init setup packet buffer */
    /* Buffer range for setup packet -> [0 ~ 0x7] */
    USBD->STBUFSEG = SETUP_BUF_BASE;

    /*****************************************************/
    /* EP0 ==> control IN endpoint, address 0 */
    USBD_CONFIG_EP(EP0, USBD_CFG_CSTALL | USBD_CFG_EPMODE_IN | 0);
    /* Buffer range for EP0 */
    USBD_SET_EP_BUF_ADDR(EP0, EP0_BUF_BASE);

    /* EP1 ==> control OUT endpoint, address 0 */
    USBD_CONFIG_EP(EP1, USBD_CFG_CSTALL | USBD_CFG_EPMODE_OUT | 0);
    /* Buffer range for EP1 */
    USBD_SET_EP_BUF_ADDR(EP1, EP1_BUF_BASE);

    /*****************************************************/
    /* EP2 ==> Bulk IN endpoint, address 2 */
    USBD_CONFIG_EP(EP2, USBD_CFG_EPMODE_IN | BULK_IN_EP);
    /* Buffer range for EP2 */
    USBD_SET_EP_BUF_ADDR(EP2, EP2_BUF_BASE);

    /* EP3 ==> Bulk Out endpoint, address 3 */
    USBD_CONFIG_EP(EP3, USBD_CFG_EPMODE_OUT | BULK_OUT_EP);
    /* Buffer range for EP3 */
    USBD_SET_EP_BUF_ADDR(EP3, EP3_BUF_BASE);

    /* trigger to receive OUT data */
    USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);

    /*****************************************************/
    g_u32BulkBuf0 = EP3_BUF_BASE;
    g_u32BulkBuf1 = EP2_BUF_BASE;
    g_TotalSectors = DATA_FLASH_STORAGE_SIZE / UDC_SECTOR_SIZE;

    /*
       Generate Mass-Storage Device serial number
       To compliant USB-IF MSC test, we must enable serial string descriptor.
       However, windows may fail to recognize the devices if PID/VID and serial number are all the same
       when plug them to Windows at the sample time.
       Therefore, we must generate different serial number for each device to avoid conflict
       when plug more then 2 MassStorage devices to Windows at the same time.

       NOTE: We use compiler predefine macro "__TIME__" to generate different number for serial
       at each build but each device here for a demo.
       User must change it to make sure all serial number is different between each device.
     */
	pu8 = (uint8_t *)gsInfo.gu8StringDesc[3];
    for(i = 0; i < 16; i+=2) 
	{
			printf("pu8[%d] = %x\r\n", (10+i*2), pu8[10+i]);
	}
}

void MSC_ClassRequest(void)
{
    // uint8_t buf[8];
	  SetupPkt_t SetupPkt;
	
    // USBD_GetSetupPacket(buf);
	  USBD_GetSetupPacket((uint8_t *) &SetupPkt);
	

    if(SetupPkt.bmRequestType.BM.Dir) 	/* request data transfer direction */
		{  
        // Device to host
        switch(SetupPkt.bRequest) 
				{
						case GET_MAX_LUN: 	// 0xFE
						{
								/* Check interface number with cfg descriptor and check wValue = 0, wLength = 1 */
							//	if((buf[4] == gsInfo.gu8ConfigDesc[LEN_CONFIG + 2]) && (buf[2] + buf[3] + buf[6] + buf[7] == 1)) 
								if((SetupPkt.wIndex.WB.L == gsInfo.gu8ConfigDesc[LEN_CONFIG + 2]) && 
									 (SetupPkt.wValue.WB.L + SetupPkt.wValue.WB.H + SetupPkt.wLength.WB.L + SetupPkt.wLength.WB.H == 1)) 	
								{
										M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = 0;
										/* Data stage */
										USBD_SET_DATA1(EP0);
										USBD_SET_PAYLOAD_LEN(EP0, 1);
										/* Status stage */
										USBD_PrepareCtrlOut(0, 0);
								} 
								else
								{	
									USBD_SET_EP_STALL(EP1); // Stall when wrong parameter
								}
								
								break;
						}
        
						default: 
						{
								/* Setup error, stall the device */
								USBD_SetStall(0);
								DBG_PRINTF("Unknow MSC req(0x%x). stall ctrl pipe\n", buf[1]);
								break;
						}
        }
    } 
		else 
		{
        // Host to device
        switch(SetupPkt.bRequest) 
				{
						case BULK_ONLY_MASS_STORAGE_RESET: 
						{
								/* Check interface number with cfg descriptor and check wValue = 0, wLength = 0 */
							//	if((buf[4] == gsInfo.gu8ConfigDesc[LEN_CONFIG + 2]) && (buf[2] + buf[3] + buf[6] + buf[7] == 0)) 
							  if((SetupPkt.wIndex.WB.L == gsInfo.gu8ConfigDesc[LEN_CONFIG + 2]) && 
									 (SetupPkt.wValue.WB.L + SetupPkt.wValue.WB.H + SetupPkt.wLength.WB.L + SetupPkt.wLength.WB.H == 0)) 
								{
										USBD_LockEpStall(0);
									  MSC_Reset();					  
									
										/* Clear ready */
										USBD->EP[EP2].CFGP |= USBD_CFGP_CLRRDY_Msk;
										USBD->EP[EP3].CFGP |= USBD_CFGP_CLRRDY_Msk;									  
									  USBD_SET_DATA0(EP2);		
									  USBD_SET_EP_BUF_ADDR(EP2, EP2_BUF_BASE);
									  USBD_SET_DATA0(EP3);		
									  USBD_SET_EP_BUF_ADDR(EP3, EP3_BUF_BASE);
									  USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);
							 } 
							 else 
							 {
									/* Stall when wrong parameter */
									USBD_SET_EP_STALL(EP1);
							 }

							/* Status stage */
							USBD_SET_DATA1(EP0);
							USBD_SET_PAYLOAD_LEN(EP0, 0);

							break;
					}
        
					default: 
					{
							// Stall
							/* Setup error, stall the device */
							USBD_SetStall(0);
							DBG_PRINTF("Unknow MSC req (0x%x). stall ctrl pipe\n", buf[1]);
							break;
					}
        }
    }
}




void MSC_SetConfig(void)
{
    // Clear stall status and ready
    USBD->EP[2].CFGP = 1;
    USBD->EP[3].CFGP = 1;
    /*****************************************************/
    /* EP2 ==> Bulk IN endpoint, address 2 */
    USBD_CONFIG_EP(EP2, USBD_CFG_EPMODE_IN | BULK_IN_EP);
    /* Buffer range for EP2 */
    USBD_SET_EP_BUF_ADDR(EP2, EP2_BUF_BASE);

    /* EP3 ==> Bulk Out endpoint, address 3 */
    USBD_CONFIG_EP(EP3, USBD_CFG_EPMODE_OUT | BULK_OUT_EP);
    /* Buffer range for EP3 */
    USBD_SET_EP_BUF_ADDR(EP3, EP3_BUF_BASE);

    /* trigger to receive OUT data */
    USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);

    USBD_LockEpStall(0);
    g_u8BotState = 0;

    DBG_PRINTF("Set config\n");

}

BOOL MSC_IsSuspended(void)
{
	if( !(USBD->ATTR&USBD_PHY_EN) && (USBD->ATTR&USBD_USB_EN) )
		return TRUE;
	
	return FALSE;
}

