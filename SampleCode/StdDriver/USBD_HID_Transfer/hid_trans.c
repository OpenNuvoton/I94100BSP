/******************************************************************************
 * @file     hid_trans.c
 * @brief    USBD HID transfer sample file
 *
 * @note
 * Copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

/*!<Includes */
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "hid_trans.h"

// HID Transfer definition
#define HIDTRANS_CMD_SIGNATURE    0x43444948
#define HIDTRANS_CMD_NONE         0x00
#define HIDTRANS_CMD_ERASE        0x71
#define HIDTRANS_CMD_READ         0xD2
#define HIDTRANS_CMD_WRITE        0xC3

typedef __packed struct
{
    uint8_t  u8Cmd;
    uint8_t  u8Size;
    uint32_t u32Arg1;
    uint32_t u32Arg2;
    uint32_t u32Signature;
    uint32_t u32Checksum;
} HIDTRANS_CMD_T;

/* Page buffer to upload/download through HID report */
static uint8_t  g_u8PageBuff[HIDTRANS_PAGESIZE] = {0};    
/* The bytes of data in g_u8PageBuff */
static uint32_t g_u32BytesInPageBuf = 0;          
/* HID transfer command handler */
static HIDTRANS_CMD_T g_sHidCmd;
/* HID process data buffer pointer */
static uint8_t* g_pu8DataBuff;

int32_t HIDTrans_CmdEraseSectors(HIDTRANS_CMD_T *pCmd)
{
	HIDTrans_EraseSector(pCmd->u32Arg1,pCmd->u32Arg2);
    pCmd->u8Cmd = HIDTRANS_CMD_NONE;
    return 0;
}
int32_t HIDTrans_CmdReadPages(HIDTRANS_CMD_T *pCmd)
{
	uint32_t u32Address = 0;
	
    if(pCmd->u32Arg2)
    {
		// Call read page function to provide input buffer data.
		HIDTrans_PrepareReadPage(&u32Address,pCmd->u32Arg1,pCmd->u32Arg2);
		// Update data to page buffer to upload
		if( u32Address == 0 )
		{
			g_pu8DataBuff = NULL;
			memset(g_u8PageBuff,0, HIDTRANS_PAGESIZE);
		}
		else
		{
			g_pu8DataBuff = (uint8_t*)u32Address;
			memcpy(g_u8PageBuff, g_pu8DataBuff, HIDTRANS_PAGESIZE);
		}
		g_u32BytesInPageBuf = HIDTRANS_PAGESIZE;
        // The signature word is used as page counter
        pCmd->u32Signature = 1;
        // Trigger HID IN
        USBD_MemCopy((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP2)), (void *)g_u8PageBuff, EP2_MAX_PKT_SIZE);
        USBD_SET_PAYLOAD_LEN(EP2, EP2_MAX_PKT_SIZE);
        g_u32BytesInPageBuf -= EP2_MAX_PKT_SIZE;
    }
    return 0;
}
int32_t HIDTrans_CmdWritePages(HIDTRANS_CMD_T *pCmd)
{
    uint32_t u32Address;

	// Call read page function to provide input buffer data.
	HIDTrans_PrepareWritePage(&u32Address,pCmd->u32Arg1,pCmd->u32Arg2);
	// Set output data buffer address
	if( u32Address == 0 )
		g_pu8DataBuff = NULL;
	else
		g_pu8DataBuff = (uint8_t*)u32Address;

    g_u32BytesInPageBuf = 0;
    pCmd->u32Signature = 0;
    return 0;
}
uint32_t HIDTrans_CalCheckSum(uint8_t *pu8buf, uint32_t u32Size)
{
    uint32_t u32Sum = 0, u32i = 0;
	
    while(u32Size--)
    {
        u32Sum += pu8buf[u32i++];
    }
    return u32Sum;
}
int32_t HIDTrans_ProcessCommand(uint8_t *pu8Buffer, uint32_t u32BufferLen)
{
    USBD_MemCopy((uint8_t *)&g_sHidCmd, pu8Buffer, u32BufferLen);

    /* Check size */
    if((g_sHidCmd.u8Size > sizeof(g_sHidCmd)) || (g_sHidCmd.u8Size > u32BufferLen))
        return -1;

    /* Check signature */
    if(g_sHidCmd.u32Signature != HIDTRANS_CMD_SIGNATURE)
        return -1;

    /* Calculate checksum & check it*/
    if(HIDTrans_CalCheckSum((uint8_t *)&g_sHidCmd, g_sHidCmd.u8Size) != g_sHidCmd.u32Checksum)
        return -1;

    switch(g_sHidCmd.u8Cmd)
    {
        case HIDTRANS_CMD_ERASE:
        {
            HIDTrans_CmdEraseSectors(&g_sHidCmd);
            break;
        }
        case HIDTRANS_CMD_READ:
        {
            HIDTrans_CmdReadPages(&g_sHidCmd);
            break;
        }
        case HIDTRANS_CMD_WRITE:
        {
            HIDTrans_CmdWritePages(&g_sHidCmd);
            break;
        }
        default:
            return -1;
    }
    return 0;
}
void HIDTrans_GetOutReport(uint8_t *pu8EpBuf, uint32_t u32Size)
{
    // Check if it is in the data phase of write command 
    if((g_sHidCmd.u8Cmd == HIDTRANS_CMD_WRITE) && (g_sHidCmd.u32Signature<g_sHidCmd.u32Arg2))
    {
        // Get data from HID OUT
        USBD_MemCopy(&g_u8PageBuff[g_u32BytesInPageBuf], pu8EpBuf, EP3_MAX_PKT_SIZE);
        g_u32BytesInPageBuf += EP3_MAX_PKT_SIZE;

        // The HOST must make sure the data is PAGE_SIZE alignment 
        if(g_u32BytesInPageBuf >= HIDTRANS_PAGESIZE)
        {
			memcpy(&g_pu8DataBuff[g_sHidCmd.u32Signature*HIDTRANS_PAGESIZE],g_u8PageBuff,HIDTRANS_PAGESIZE);
            g_sHidCmd.u32Signature++;
            // Write command complete! 
            if(g_sHidCmd.u32Signature >= g_sHidCmd.u32Arg2)
			{
				HIDTrans_GetWriteData((uint32_t)g_pu8DataBuff,g_sHidCmd.u32Arg2);
                g_sHidCmd.u8Cmd = HIDTRANS_CMD_NONE;
            }
			g_u32BytesInPageBuf = 0;
        }
    }
    else
    {
        // Check and process the command packet 
        HIDTrans_ProcessCommand(pu8EpBuf, u32Size);
    }
}
void HIDTrans_SetInReport(void)
{
    uint8_t* ptr;

    // Check if it is in data phase of read command
    if(g_sHidCmd.u8Cmd == HIDTRANS_CMD_READ)
    {
        // Process the data phase of read command
        if((g_sHidCmd.u32Signature >= g_sHidCmd.u32Arg2) && (g_u32BytesInPageBuf == 0))
            g_sHidCmd.u8Cmd = HIDTRANS_CMD_NONE;
        else
        {
            if(g_u32BytesInPageBuf == 0)
            {
				// The previous page has sent out. Read new page to page buffer 
				if( g_pu8DataBuff == NULL )
					memset(g_u8PageBuff,0, HIDTRANS_PAGESIZE);
				else
					memcpy(g_u8PageBuff, &g_pu8DataBuff[g_sHidCmd.u32Signature*HIDTRANS_PAGESIZE], HIDTRANS_PAGESIZE);
				g_u32BytesInPageBuf = HIDTRANS_PAGESIZE;
                // Update the page counter
                g_sHidCmd.u32Signature++;
            }

            // Prepare the data for next HID IN transfer
            ptr = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP2));
            USBD_MemCopy(ptr, (void *)&g_u8PageBuff[HIDTRANS_PAGESIZE - g_u32BytesInPageBuf], EP2_MAX_PKT_SIZE);
            USBD_SET_PAYLOAD_LEN(EP2, EP2_MAX_PKT_SIZE);
            g_u32BytesInPageBuf -= EP2_MAX_PKT_SIZE;
        }
    }
}
// Interrupt IN handler 
void EP2_Handler(void)  
{
    HIDTrans_SetInReport();
}
// Interrupt OUT handler
void EP3_Handler(void)  
{
    uint8_t *ptr;
    ptr = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP3));
    HIDTrans_GetOutReport(ptr, USBD_GET_PAYLOAD_LEN(EP3));
    USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);
}
void USBD_IRQHandler(void)
{
    uint32_t u32IntSts = USBD_GET_INT_FLAG();
    uint32_t u32State = USBD_GET_BUS_STATE();

//------------------------------------------------------------------
	if(u32IntSts & USBD_INTSTS_FLDET)
	{
		// Floating detect
		USBD_CLR_INT_FLAG(USBD_INTSTS_FLDET);

		if(USBD_IS_ATTACHED())
		{
			// Enable GPB15(VBUS) pull down state to solute suspend event issue.
			GPIO_EnablePullState(PB,BIT15,GPIO_PUSEL_PULL_DOWN); 	
			/* USB Plug In */
			USBD_ENABLE_USB();
		}
		else
		{
			// Disable GPB15 pull down state.
			GPIO_DisablePullState(PB,BIT15); 
			/* USB Un-plug */
			USBD_DISABLE_USB();
		}
	}

//------------------------------------------------------------------
	if ( u32IntSts & USBD_INTSTS_SOFIF_Msk )
	{
		// Clear event flag 
		USBD_CLR_INT_FLAG(USBD_INTSTS_SOFIF_Msk);
	}

//------------------------------------------------------------------
	if(u32IntSts & USBD_INTSTS_BUS)
	{
			/* Clear event flag */
			USBD_CLR_INT_FLAG(USBD_INTSTS_BUS);

			if(u32State & USBD_STATE_USBRST)
			{
					/* Bus reset */
					USBD_ENABLE_USB();
					USBD_SwReset();
			}
			if(u32State & USBD_STATE_SUSPEND)
			{
					/* Enable USB but disable PHY */
					USBD_DISABLE_PHY(); 
			}
			if(u32State & USBD_STATE_RESUME)
			{
					/* Enable USB and enable PHY */
					USBD_ENABLE_USB();
			}
	}

//------------------------------------------------------------------
	if(u32IntSts & USBD_INTSTS_USB)
	{
		// USB event
		if(u32IntSts & USBD_INTSTS_SETUP)
		{
			// Setup packet
			/* Clear event flag */
			USBD_CLR_INT_FLAG(USBD_INTSTS_SETUP);

			/* Clear the data IN/OUT ready flag of control end-points */
			USBD_STOP_TRANSACTION(EP0);
			USBD_STOP_TRANSACTION(EP1);

			USBD_ProcessSetupPacket();
		}
		// EP events
		if(u32IntSts & USBD_INTSTS_EP0)
		{
			/* Clear event flag */
			USBD_CLR_INT_FLAG(USBD_INTSTS_EP0);
			// Control IN
			USBD_CtrlIn();
		}
		if(u32IntSts & USBD_INTSTS_EP1)
		{
			/* Clear event flag */
			USBD_CLR_INT_FLAG(USBD_INTSTS_EP1);
			// Control OUT
			USBD_CtrlOut();
		}
		if(u32IntSts & USBD_INTSTS_EP2)
		{
			/* Clear event flag */
			USBD_CLR_INT_FLAG(USBD_INTSTS_EP2);
			// Interrupt IN
			EP2_Handler();
		}
		if(u32IntSts & USBD_INTSTS_EP3)
		{
			/* Clear event flag */
			USBD_CLR_INT_FLAG(USBD_INTSTS_EP3);
			// Interrupt OUT
			EP3_Handler();
		}
	}
	
	/* clear unknown event */
	USBD_CLR_INT_FLAG(u32IntSts);
}
void HIDTrans_Initiate(void)
{
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
    /* EP2 ==> Interrupt IN endpoint, address 1 */
    USBD_CONFIG_EP(EP2, USBD_CFG_EPMODE_IN | INT_IN_EP_NUM);
    /* Buffer range for EP2 */
    USBD_SET_EP_BUF_ADDR(EP2, EP2_BUF_BASE);

    /* EP3 ==> Interrupt OUT endpoint, address 2 */
    USBD_CONFIG_EP(EP3, USBD_CFG_EPMODE_OUT | INT_OUT_EP_NUM);
    /* Buffer range for EP3 */
    USBD_SET_EP_BUF_ADDR(EP3, EP3_BUF_BASE);
    /* trigger to receive OUT data */
    USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);
}
void HIDTrans_ClassRequest(void)
{
    uint8_t buf[8];

    USBD_GetSetupPacket(buf);

	// request data transfer direction 
    if(buf[0] & 0x80) 
    {
        // Device to host
        switch(buf[1])
        {
            case GET_REPORT:
            case GET_IDLE:
            case GET_PROTOCOL:
            default:
            {
                USBD_SetStall(0);
                break;
            }
        }
    }
    else
    {
        // Host to device
        switch(buf[1])
        {
            case SET_REPORT:
            {
                if(buf[3] == 3)
                {
                    // Request Type = Feature
                    USBD_SET_DATA1(EP1);
                    USBD_SET_PAYLOAD_LEN(EP1, 0);
                }
                break;
            }
            case SET_IDLE:
            {
                /* Status stage */
                USBD_SET_DATA1(EP0);
                USBD_SET_PAYLOAD_LEN(EP0, 0);
                break;
            }
            case SET_PROTOCOL:
            default:
            {

                USBD_SetStall(0);
                break;
            }
        }
    }
}
__weak void HIDTrans_EraseSector(uint32_t u32StartSector,uint32_t u32Sectors)
{
}
__weak void HIDTrans_ReadPage(uint32_t* pu32Address,uint32_t u32StartPage,uint32_t u32Pages)
{
}
__weak void HIDTrans_WritePage(uint32_t* pu32Address,uint32_t u32StartPage,uint32_t u32Pages)
{
}
__weak void HIDTrans_GetWriteData(uint32_t u32Address,uint32_t u32Pages)
{
}
