/******************************************************************************
 * @file     hid_composite_device.c
 * @brief   USBD driver Sample file
 * @version  1.0.0
 * @date     25, Nov, 2019
 *
 * @note
 * Copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

/*!<Includes */
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "hid_request.h"

uint8_t volatile g_u8EP2Ready = 0;
uint8_t volatile g_u8EP3Ready = 0;
uint8_t volatile g_u8EP4Ready = 0;
uint8_t volatile g_u8EP5Ready = 0;
uint8_t volatile g_u8EP6Ready = 0;
uint8_t volatile g_u8EP7Ready = 0;
uint8_t volatile g_u8EP8Ready = 0;
uint8_t volatile g_u8EP9Ready = 0;
uint8_t volatile g_u8EP10Ready = 0;
uint8_t volatile g_u8EP11Ready = 0;


void USBD_IRQHandler(void)
{
    uint32_t u32IntSts = USBD_GET_INT_FLAG();		// Read USBD_INTSTS register 
    uint32_t u32State = USBD_GET_BUS_STATE();		// reset, suspend, resume, timeout flags 

//------------------------------------------------------------------
    if (u32IntSts & USBD_INTSTS_FLDET) 
	{
        // Floating detect
        USBD_CLR_INT_FLAG(USBD_INTSTS_FLDET);

        if (USBD_IS_ATTACHED()) 
		{
            // USB Plug In 
            USBD_ENABLE_USB();
        } 
		else 
		{
            // USB Un-plug 
            USBD_DISABLE_USB();
        }
    }
	
//------------------------------------------------------------------
	if (u32IntSts & USBD_INTSTS_WAKEUP) 	
	{
		/* Clear event flag */
		USBD_CLR_INT_FLAG(USBD_INTSTS_WAKEUP);
	}
			
//------------------------------------------------------------------
	if ( u32IntSts & USBD_INTSTS_SOFIF_Msk )		
	{
		/* Clear event flag */
		USBD_CLR_INT_FLAG(USBD_INTSTS_SOFIF_Msk);			
	}
			
//------------------------------------------------------------------
    if (u32IntSts & USBD_INTSTS_BUS) 
	{
        /* Clear event flag */
        USBD_CLR_INT_FLAG(USBD_INTSTS_BUS);

        if (u32State & USBD_STATE_USBRST) 
		{
            /* Bus reset */
            USBD_ENABLE_USB();
            USBD_SwReset();
        }
        if (u32State & USBD_STATE_SUSPEND) 
		{
            /* Enable USB but disable PHY */
			USBD_DISABLE_PHY();
        }	
        if (u32State & USBD_STATE_RESUME) 
		{
            /* Enable USB and enable PHY */
            USBD_ENABLE_USB();		  
        }
    }

//------------------------------------------------------------------
    if (u32IntSts & USBD_INTSTS_USB) 
	{
        // USB event
        if (u32IntSts & USBD_INTSTS_SETUP) 
		{
			// Setup packet
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_SETUP);

            /* Clear the data IN/OUT ready flag of control end-points */
            USBD_STOP_TRANSACTION(CTRL_IN_EP);
            USBD_STOP_TRANSACTION(CTRL_OUT_EP);

            USBD_ProcessSetupPacket();		
        }

		// EP events
		if (u32IntSts & USBD_INTSTS_EP0) 
		{
			/* Clear event flag */
			USBD_CLR_INT_FLAG(USBD_INTSTS_EP0);
			// control IN
			USBD_CtrlIn();
		}
		if (u32IntSts & USBD_INTSTS_EP1) 
		{
			/* Clear event flag */
			USBD_CLR_INT_FLAG(USBD_INTSTS_EP1);
			// control OUT
			USBD_CtrlOut();
		}
        if (u32IntSts & USBD_INTSTS_EP2) 
		{
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP2);
            // Interrupt IN
            EP2_Handler();
        }
        if (u32IntSts & USBD_INTSTS_EP3) 
		{
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP3);
			EP3_Handler();
        }
        if (u32IntSts & USBD_INTSTS_EP4) 
		{
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP4);
			// Interrupt IN
            EP4_Handler();
        }
        if (u32IntSts & USBD_INTSTS_EP5) 
				{
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP5);
						EP5_Handler();
        }
        if (u32IntSts & USBD_INTSTS_EP6) 
				{
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP6);
						EP6_Handler();
        }
        if (u32IntSts & USBD_INTSTS_EP7) 
				{
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP7);
						EP7_Handler();
        }
		if (u32IntSts & USBD_INTSTS_EP8) 
		{
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP8);
			EP8_Handler();
        }
		if (u32IntSts & USBD_INTSTS_EP9) 
		{
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP9);
			EP9_Handler();
        }
		if (u32IntSts & USBD_INTSTS_EP10) 
		{
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP10);
			EP10_Handler();
        }
		if (u32IntSts & USBD_INTSTS_EP11) 
		{
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP11);
			EP11_Handler();
        }
    }
}

void EP2_Handler(void)  /* Interrupt handler */
{
  g_u8EP2Ready = 1;	
}

void EP3_Handler(void)  /* Interrupt handler */
{
	g_u8EP3Ready = 1;	 
}

void EP4_Handler(void)  /* Interrupt handler */
{
	g_u8EP4Ready = 1;	
}

void EP5_Handler(void)  /* Interrupt handler */
{
	g_u8EP5Ready = 1;	
}

void EP6_Handler(void)  /* Interrupt handler */
{
	g_u8EP6Ready = 1;	
}

void EP7_Handler(void)  /* Interrupt handler */
{
	g_u8EP7Ready = 1;	
}

void EP8_Handler(void)  /* Interrupt handler */
{
	g_u8EP8Ready = 1;	
}

void EP9_Handler(void)  /* Interrupt handler */
{
	g_u8EP9Ready = 1;	
}

void EP10_Handler(void)  /* Interrupt handler */
{
	g_u8EP10Ready = 1;	
}

void EP11_Handler(void)  /* Interrupt handler */
{
	g_u8EP11Ready = 1;	
}

/*--------------------------------------------------------------------------*/
/**
  * @brief  USBD Endpoint Config.
  * @param  None.
  * @retval None.
  */
void HID_Init(void)
{
    /* Init setup packet buffer */
    /* Buffer range for setup packet -> [0 ~ 0x7] */
    USBD->STBUFSEG = SETUP_BUF_BASE;

    /*****************************************************/
    /* EP0 ==> control IN endpoint, address 0 */
    USBD_CONFIG_EP(CTRL_IN_EP, USBD_CFG_CSTALL | USBD_CFG_EPMODE_IN | 0);
    /* Buffer range for EP0 */
    USBD_SET_EP_BUF_ADDR(CTRL_IN_EP, EP0_BUF_BASE);

    /* EP1 ==> control OUT endpoint, address 0 */
    USBD_CONFIG_EP(CTRL_OUT_EP, USBD_CFG_CSTALL | USBD_CFG_EPMODE_OUT | 0);
    /* Buffer range for EP1 */
    USBD_SET_EP_BUF_ADDR(CTRL_OUT_EP, EP1_BUF_BASE);
	   
    /*****************************************************/
	/* EP2 ==> Interrupt IN endpoint, address 2 */
    USBD_CONFIG_EP(EP2, USBD_CFG_EPMODE_IN | 0x02);
    /* Buffer range for EP2 */
    USBD_SET_EP_BUF_ADDR(EP2, EP2_BUF_BASE);
	
	/* EP3 ==> Interrupt IN endpoint, address 3 */
    USBD_CONFIG_EP(EP3, USBD_CFG_EPMODE_IN | 0x03);
    /* Buffer range for EP3 */
    USBD_SET_EP_BUF_ADDR(EP3, EP3_BUF_BASE);
				
	/* EP4 ==> Interrupt IN endpoint, address 4 */
    USBD_CONFIG_EP(EP4, USBD_CFG_EPMODE_IN | 0x04);
    /* Buffer range for EP4 */
    USBD_SET_EP_BUF_ADDR(EP4, EP4_BUF_BASE);
	
	/* EP5 ==> Interrupt IN endpoint, address 5 */
    USBD_CONFIG_EP(EP5, USBD_CFG_EPMODE_IN | 0x05);
    /* Buffer range for EP5 */
    USBD_SET_EP_BUF_ADDR(EP5, EP5_BUF_BASE);
		
	/* EP6 ==> Interrupt IN endpoint, address 6 */
    USBD_CONFIG_EP(EP6, USBD_CFG_EPMODE_IN | 0x06);
    /* Buffer range for EP6 */
    USBD_SET_EP_BUF_ADDR(EP6, EP6_BUF_BASE);
	
	/* EP7 ==> Interrupt IN endpoint, address 7 */
    USBD_CONFIG_EP(EP7, USBD_CFG_EPMODE_IN | 0x07);
    /* Buffer range for EP7 */
    USBD_SET_EP_BUF_ADDR(EP7, EP7_BUF_BASE);
		
	/* EP8 ==> Interrupt IN endpoint, address 8 */
    USBD_CONFIG_EP(EP8, USBD_CFG_EPMODE_IN | 0x08);
    /* Buffer range for EP8 */
    USBD_SET_EP_BUF_ADDR(EP8, EP8_BUF_BASE);
	
	/* EP9 ==> Interrupt IN endpoint, address 9 */
    USBD_CONFIG_EP(EP9, USBD_CFG_EPMODE_IN | 0x09);
    /* Buffer range for EP9 */
    USBD_SET_EP_BUF_ADDR(EP9, EP9_BUF_BASE);
		
	/* EP10 ==> Interrupt IN endpoint, address 10 */
    USBD_CONFIG_EP(EP10, USBD_CFG_EPMODE_IN | 0x0A);
    /* Buffer range for EP10 */
    USBD_SET_EP_BUF_ADDR(EP10, EP10_BUF_BASE);
		
	/* EP11 ==> Interrupt IN endpoint, address 11 */
    USBD_CONFIG_EP(EP11, USBD_CFG_EPMODE_IN | 0x0B);
    /* Buffer range for EP11 */
    USBD_SET_EP_BUF_ADDR(EP11, EP11_BUF_BASE);
	
   /* Start to send IN data */
    g_u8EP2Ready = 1;
}

void HID_ClassRequest(void)
{
    uint8_t buf[8]; 
	 		
    USBD_GetSetupPacket(buf);
	
    if (buf[0] & 0x80) 		/* request data transfer direction */
	{ 
		// Device to host
		switch (buf[1] ) 
		{
			case GET_REPORT: 
			{
				if(buf[3] == HID_RPT_TYPE_INPUT) 
				{
					/* Report Type = input */
					//DBG_PRINTF(" - Input\n");
				} 
				else if(buf[3] == HID_RPT_TYPE_FEATURE) 
				{	
					/* Request Type = Feature */
					/* report ID is 2 */
					/* contact count maximum is 2 */
					M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(CTRL_IN_EP)) = 2;
					M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(CTRL_IN_EP) + 1) = 2;
					/* Data stage */
					USBD_SET_DATA1(CTRL_IN_EP);
					USBD_SET_PAYLOAD_LEN(CTRL_IN_EP, 2);
					/* Status stage */
					USBD_PrepareCtrlOut(0,0);
				} 
				else 
				{
					// DBG_PRINTF(" - Unknown\n");
					/* Setup error, stall the device */
					USBD_SetStall(0);
				}
				break;
			}
			case GET_IDLE:
			case GET_PROTOCOL:
			default: 
			{
				/* Setup error, stall the device */
				USBD_SetStall(0);
				break;
			}
		}
    } 
	else 
	{
		// Host to device
		switch (buf[1]) 
		{
			case SET_REPORT: 
			{
				if (buf[3] == 2) 	
				{
					/* Request Type = OUTPUT */
					USBD_SET_DATA1(CTRL_OUT_EP);
					USBD_SET_PAYLOAD_LEN(CTRL_OUT_EP, 0);
				} 
				else if (buf[3] == 3) 
				{
					/* Request Type = Feature */
					USBD_SET_DATA1(CTRL_OUT_EP);
					USBD_SET_PAYLOAD_LEN(CTRL_OUT_EP, 0);
				}
				break;
			}
			case SET_IDLE: 
			{
				/* Status stage */
				USBD_SET_DATA1(CTRL_IN_EP);
				USBD_SET_PAYLOAD_LEN(CTRL_IN_EP, 0);
				break;
			}
			case SET_PROTOCOL:
			default: 
			{
				// Stall
				/* Setup error, stall the device */
				USBD_SetStall(0);
				break;
			}
		}
    }
}


/* two lines demo in Paint */
unsigned int ReportCount = 0;
unsigned char bIsX1Send04 = 0, bIsX2Send04 = 0;
/*
 add decrription for mouse operation *** 
*/
void HID_UpdateTouchData(void)
{
    uint8_t *buf;
    static unsigned short X1 = 0x01f0, Y1 = 0x0100;

    if(g_u8EP2Ready) 
	{
        buf = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP2));

        /* Report ID 1 */
        buf[0] = 1;
        buf[2] = 0;
        buf[8] = 1;

        ReportCount++;

        if ((X1 >= 0x0200) && (X1 <= 0x0400)) // touchDown
		{ 
            bIsX1Send04 = 1;

            buf[1] = 0x07;
            buf[3] = X1 & 0xff;
            buf[4] = (X1 >> 8) & 0xff;
            buf[5] = Y1 & 0xff;
            buf[6] = (Y1 >> 8) & 0xff;
            buf[7] = 0x07;
            buf[9] = X1 & 0xff;
            buf[10] = (X1 >> 8) & 0xff;
            buf[11] = (Y1+0x20) & 0xff;
            buf[12] = ((Y1+0x20) >> 8) & 0xff;
            buf[13] = 2;
        } 
		else if (bIsX1Send04) // touchUp
		{ 
            bIsX1Send04 = 0;
            buf[1] = 0x04;
            buf[3] = 0x00;
            buf[4] = 0x04;
            buf[5] = Y1 & 0xff;
            buf[6] = (Y1 >> 8) & 0xff;
            buf[7] = 0x04;
            buf[9] = 0x00;
            buf[10] = 0x04;
            buf[11] = (Y1+0x20) & 0xff;
            buf[12] = ((Y1+0x20) >> 8) & 0xff;
            buf[13] = 2;

            if (Y1 == 0x0100) 
			{
                buf[5] = 0xE0;
                buf[6] = 0x02;
                buf[11] = 0x00;
                buf[12] = 0x03;
            } 
			else 
			{
                buf[5] = (Y1-0x50) & 0xff;
                buf[6] = ((Y1-0x50) >> 8) & 0xff;
                buf[11] = ((Y1+0x20)-0x50) & 0xff;
                buf[12] = (((Y1+0x20)-0x50) >> 8) & 0xff;
            }
        } 
		else 
		{
            buf[1] = 0;
            buf[3] = 0;
            buf[4] = 0;
            buf[5] = 0;
            buf[6] = 0;
            buf[7] = 0;
            buf[9] = 0;
            buf[10] = 0;
            buf[11] = 0;
            buf[12] = 0;
            buf[13] = 0;
        }

        if ((ReportCount % 6) == 0)
            X1 += 0x3;

        if (X1 > 0x0400) 
		{
            X1 = 0x01f0;
            Y1 += 0x50;
            if (Y1 > 0x0300)
                Y1 = 0x0100;
        }

        g_u8EP2Ready = 0;
        USBD_SET_PAYLOAD_LEN(EP2, 14);
    }
}

