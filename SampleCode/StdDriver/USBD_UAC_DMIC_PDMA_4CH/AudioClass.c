/******************************************************************************
 * @file     MassStorage.c
 * @brief    USBD driver Sample file
 *
 * @note
 * Copyright (C) 2018 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

/*!<Includes */
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "usbd_audio.h"
#include "audioclass.h"
#include "pdma_config.h"
#include "uac_buffer.h"

#if 0
	#define DBG_PRINTF      printf
#else
	#define DBG_PRINTF(...)
#endif


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
	DB16  			wValue;
	DB16  			wIndex;
	DB16  			wLength;
} SetupPkt_t;


/*Get MAX or MIN value*/
#define GetMax( x1, x2 ) ( ( x1 ) > ( x2 ) ? ( x1 ) : ( x2 ) )
#define GetMin( x1, x2 ) ( ( x1 ) > ( x2 ) ? ( x2 ) : ( x1 ) )


/*--------------------------------------------------------------------------*/
// Extern Functions
extern void DMIC_Init(uint32_t u32SampleRate, uint8_t ChannelNum);
extern void DPWM_Init(uint32_t u32SampleRate);
extern void PDMA_Init(PDMA_CONFIG_T *pdma_ctl);

//----------------------------------------------------------------------------
//  Global variables for Control Pipe
//---------------------------------------------------------------------------- 
volatile uint8_t g_u8Update_USB_Buffer_Flag;
volatile uint8_t g_u8AudioSpeakerState = 0;
volatile uint8_t g_u8AudioMicState = 0;

// PlayBack Buffer Control Variable
// Extern
extern volatile uint16_t g_u16PDMA2DPWM_Bufflen;	
// Global
volatile uint32_t g_au32USB2PDMA_Buff[MAX_USB_BUFFER_LEN];
volatile uint32_t g_au32PDMA2DPWM_Buff[2][MAX_USB_BUFFER_LEN];
volatile uint8_t g_u8DPWM_Buff_Index = 0;
volatile uint16_t g_u16PlayBack_Read_Ptr = 0;
volatile uint16_t g_u16PlayBack_Write_Ptr = 0;
volatile uint16_t g_u16PlayBack_Ptrs_Distance = 0;
volatile uint16_t g_u16PayLoadLen;

// Record Buffer Control Variable
// Global
volatile uint32_t g_au32PDMA2USB_RingBuff[DMIC_RING_BUFFER_LEN_4CH];

/*--------------------------------------------------------------------------*/
void USBD_IRQHandler(void)
{
    uint32_t u32IntSts = USBD_GET_INT_FLAG();
    uint32_t u32State = USBD_GET_BUS_STATE();

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

		
		if ( u32IntSts & USBD_INTSTS_SOFIF_Msk )
		{
				/* Clear event flag */
				USBD_CLR_INT_FLAG(USBD_INTSTS_SOFIF_Msk);
		}	
		
    if(u32IntSts & USBD_INTSTS_BUS) 
	{
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


    if(u32IntSts & USBD_INTSTS_USB) {

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

        if(u32IntSts & USBD_INTSTS_EP2) 
		{
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP2);
            EP2_Handler();
        }

        if(u32IntSts & USBD_INTSTS_EP3) 
		{
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP3);
            EP3_Handler();
        }

        if(u32IntSts & USBD_INTSTS_EP4) 
		{
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP4);
			EP4_Handler();
        }

        if(u32IntSts & USBD_INTSTS_EP5) 
				{
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP5);
        }

        /////////////////////////////////////////////////////////
        if(u32IntSts & USBD_INTSTS_EP6) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP6);
            EP6_Handler();
        }

        if(u32IntSts & USBD_INTSTS_EP7) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP7);
            EP7_Handler();
        }
        
        if(u32IntSts & USBD_INTSTS_EP8) {
            /* Clear event flag */
            USBD_CLR_INT_FLAG(USBD_INTSTS_EP8);
            EP8_Handler();
        }
        
        /////////////////////////////////////////////////////////

				
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
	int i;
	uint16_t u16distance;
	uint8_t *pu8buf;
	uint8_t u8EP2_Byte_Count;
				
	if ( UAC_RECODER.u8AudioRecInterfConfig != REC_IFNUM_1CH )
	{
			USBD_SET_PAYLOAD_LEN(EP2, 0);
			return;
	}
    
	if (UAC_RECODER.u8AudioMicState == UAC_BUSY_AUDIO_RECORD)
	{
		// This section calculate the data number in UAC_RingBuffer.
		// And set the next transfer size depends on buffer threshold.
		if ( UAC_RECODER.u16UAC_Buff_WriteIndex >= UAC_RECODER.u16UAC_Buff_ReadIndex ) 
		{
			u16distance = UAC_RECODER.u16UAC_Buff_WriteIndex - UAC_RECODER.u16UAC_Buff_ReadIndex;	
		}
		else 
		{   
			u16distance = (DMIC_RING_BUFFER_LEN_1CH - UAC_RECODER.u16UAC_Buff_ReadIndex) + UAC_RECODER.u16UAC_Buff_WriteIndex;	
		}
		
		if ( u16distance >= DMIC_BUFF_UPPER_THRE_1CH ) 
		{
			u8EP2_Byte_Count = DMIC_EP2BC_BASE + DMIC_EP2BC_ADD;		
		}
		else if ( u16distance < DMIC_BUFF_LOWER_THRE_1CH ) 
		{
			u8EP2_Byte_Count = DMIC_EP2BC_BASE - DMIC_EP2BC_SUB;		
		}
		else 
		{
			u8EP2_Byte_Count = DMIC_EP2BC_BASE;
		}
		
		pu8buf = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP2));
		
		for ( i= 0 ; i < (u8EP2_Byte_Count/3) ; i++)
		{
			if(UAC_RECODER.u16UAC_Buff_ReadIndex == UAC_RECODER.u16UAC_Buff_WriteIndex)
			{
				pu8buf[i*3+0] = 0x00;
				pu8buf[i*3+1] = 0x00;
				pu8buf[i*3+2] = 0x00;
			}
			else
			{
				if (++UAC_RECODER.u16UAC_Buff_ReadIndex >= DMIC_RING_BUFFER_LEN_1CH)       
					UAC_RECODER.u16UAC_Buff_ReadIndex = 0;

				pu8buf[i*3+0] = (g_au32PDMA2USB_RingBuff[UAC_RECODER.u16UAC_Buff_ReadIndex] >>  0) & 0xff;
				pu8buf[i*3+1] = (g_au32PDMA2USB_RingBuff[UAC_RECODER.u16UAC_Buff_ReadIndex] >>  8) & 0xff;
				pu8buf[i*3+2] = (g_au32PDMA2USB_RingBuff[UAC_RECODER.u16UAC_Buff_ReadIndex] >> 16) & 0xff;
			}
		}
						
		//USBD_SET_PAYLOAD_LEN(EP2, EP2_MAX_PKT_SIZE);
		USBD_SET_PAYLOAD_LEN(EP2, u8EP2_Byte_Count);
	}
	else
	{	
		USBD_SET_PAYLOAD_LEN(EP2, 0);
	}
	
	return;
}

void EP6_Handler(void)
{	
	int i;
	uint16_t u16distance;
	uint8_t *pu8buf;
	uint8_t u8EP6_Byte_Count;
				
	if ( UAC_RECODER.u8AudioRecInterfConfig != REC_IFNUM_2CH )
	{
			USBD_SET_PAYLOAD_LEN(EP6, 0);
			return;
	}
    
	if (UAC_RECODER.u8AudioMicState == UAC_BUSY_AUDIO_RECORD)
	{
		// This section calculate the data number in UAC_RingBuffer.
		// And set the next transfer size depends on buffer threshold.
		if ( UAC_RECODER.u16UAC_Buff_WriteIndex >= UAC_RECODER.u16UAC_Buff_ReadIndex ) 
		{
			u16distance = UAC_RECODER.u16UAC_Buff_WriteIndex - UAC_RECODER.u16UAC_Buff_ReadIndex;	
		}
		else 
		{   
			u16distance = (DMIC_RING_BUFFER_LEN_2CH - UAC_RECODER.u16UAC_Buff_ReadIndex) + UAC_RECODER.u16UAC_Buff_WriteIndex;	
		}
		
		if ( u16distance >= DMIC_BUFF_UPPER_THRE_2CH ) 
		{
			u8EP6_Byte_Count = DMIC_EP6BC_BASE + DMIC_EP6BC_ADD;		
		}
		else if ( u16distance < DMIC_BUFF_LOWER_THRE_2CH ) 
		{
			u8EP6_Byte_Count = DMIC_EP6BC_BASE - DMIC_EP6BC_SUB;		
		}
		else 
		{
			u8EP6_Byte_Count = DMIC_EP6BC_BASE;
		}
		
		pu8buf = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP6));
		
		for ( i= 0 ; i < (u8EP6_Byte_Count/3) ; i++)
		{
			if(UAC_RECODER.u16UAC_Buff_ReadIndex == UAC_RECODER.u16UAC_Buff_WriteIndex)
			{
				pu8buf[i*3+0] = 0x00;
				pu8buf[i*3+1] = 0x00;
				pu8buf[i*3+2] = 0x00;
			}
			else
			{
				if (++UAC_RECODER.u16UAC_Buff_ReadIndex >= DMIC_RING_BUFFER_LEN_2CH)       
					UAC_RECODER.u16UAC_Buff_ReadIndex = 0;				
					
				pu8buf[i*3+0] = (g_au32PDMA2USB_RingBuff[UAC_RECODER.u16UAC_Buff_ReadIndex] >>  0) & 0xff;
				pu8buf[i*3+1] = (g_au32PDMA2USB_RingBuff[UAC_RECODER.u16UAC_Buff_ReadIndex] >>  8) & 0xff;
				pu8buf[i*3+2] = (g_au32PDMA2USB_RingBuff[UAC_RECODER.u16UAC_Buff_ReadIndex] >> 16) & 0xff;
			}
		}
							
		USBD_SET_PAYLOAD_LEN(EP6, u8EP6_Byte_Count);	
	}
	else
	{	
		USBD_SET_PAYLOAD_LEN(EP6, 0);
	}
	
	return;
}

void EP7_Handler(void)
{	
	int i;
	uint16_t u16distance;
	uint8_t *pu8buf;
	uint8_t u8EP7_Byte_Count;
				
	if ( UAC_RECODER.u8AudioRecInterfConfig != REC_IFNUM_3CH )
	{
			USBD_SET_PAYLOAD_LEN(EP7, 0);
			return;
	}
		
	if (UAC_RECODER.u8AudioMicState == UAC_BUSY_AUDIO_RECORD)
	{
		// This section calculate the data number in UAC_RingBuffer.
		// And set the next transfer size depends on buffer threshold.
		if ( UAC_RECODER.u16UAC_Buff_WriteIndex >= UAC_RECODER.u16UAC_Buff_ReadIndex ) 
		{
			u16distance = UAC_RECODER.u16UAC_Buff_WriteIndex - UAC_RECODER.u16UAC_Buff_ReadIndex;	
		}
		else 
		{   
			u16distance = (DMIC_RING_BUFFER_LEN_3CH - UAC_RECODER.u16UAC_Buff_ReadIndex) + UAC_RECODER.u16UAC_Buff_WriteIndex;	
		}
		
		if ( u16distance >= DMIC_BUFF_UPPER_THRE_3CH ) 
		{
			u8EP7_Byte_Count = DMIC_EP7BC_BASE + DMIC_EP7BC_ADD;		
		}
		else if ( u16distance < DMIC_BUFF_LOWER_THRE_3CH ) 
		{
			u8EP7_Byte_Count = DMIC_EP7BC_BASE - DMIC_EP7BC_SUB;		
		}
		else 
		{
			u8EP7_Byte_Count = DMIC_EP7BC_BASE;
		}
		
		pu8buf = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP7));
		
		for ( i= 0 ; i < (u8EP7_Byte_Count/3) ; i++)
		{
			if(UAC_RECODER.u16UAC_Buff_ReadIndex == UAC_RECODER.u16UAC_Buff_WriteIndex)
			{
				pu8buf[i*3+0] = 0x00;
				pu8buf[i*3+1] = 0x00;
				pu8buf[i*3+2] = 0x00;
			}
			else
			{
				if (++UAC_RECODER.u16UAC_Buff_ReadIndex >= DMIC_RING_BUFFER_LEN_3CH)       
					UAC_RECODER.u16UAC_Buff_ReadIndex = 0;
				
				pu8buf[i*3+0] = (g_au32PDMA2USB_RingBuff[UAC_RECODER.u16UAC_Buff_ReadIndex] >>  0) & 0xff;
				pu8buf[i*3+1] = (g_au32PDMA2USB_RingBuff[UAC_RECODER.u16UAC_Buff_ReadIndex] >>  8) & 0xff;
				pu8buf[i*3+2] = (g_au32PDMA2USB_RingBuff[UAC_RECODER.u16UAC_Buff_ReadIndex] >> 16) & 0xff;
			}
		}
						
		USBD_SET_PAYLOAD_LEN(EP7, u8EP7_Byte_Count);
	}
	else
	{	
		USBD_SET_PAYLOAD_LEN(EP7, 0);
	}
	
	return;
}

void EP8_Handler(void)
{	
	int i;
	uint16_t u16distance;
	uint8_t *pu8buf;
	uint8_t u8EP8_Byte_Count;
				
	if ( UAC_RECODER.u8AudioRecInterfConfig != REC_IFNUM_4CH )
	{
		USBD_SET_PAYLOAD_LEN(EP8, 0);
		return;
	}
	
	if (UAC_RECODER.u8AudioMicState == UAC_BUSY_AUDIO_RECORD)
	{
		// This section calculate the data number in UAC_RingBuffer.
		// And set the next transfer size depends on buffer threshold.
		if ( UAC_RECODER.u16UAC_Buff_WriteIndex >= UAC_RECODER.u16UAC_Buff_ReadIndex ) 
		{
				u16distance = UAC_RECODER.u16UAC_Buff_WriteIndex - UAC_RECODER.u16UAC_Buff_ReadIndex;	
		}
		else 
		{   
			 u16distance = (DMIC_RING_BUFFER_LEN_4CH - UAC_RECODER.u16UAC_Buff_ReadIndex) + UAC_RECODER.u16UAC_Buff_WriteIndex;	
		}
		
		if ( u16distance >= DMIC_BUFF_UPPER_THRE_4CH ) 
		{
				u8EP8_Byte_Count = DMIC_EP8BC_BASE + DMIC_EP8BC_ADD;		
		}
		else if ( u16distance < DMIC_BUFF_LOWER_THRE_4CH ) 
		{
				u8EP8_Byte_Count = DMIC_EP8BC_BASE - DMIC_EP8BC_SUB;		
		}
		else 
		{
				u8EP8_Byte_Count = DMIC_EP8BC_BASE;
		}
		
		pu8buf = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP8));
		
		for ( i= 0 ; i < (u8EP8_Byte_Count/3) ; i++)
		{ 
			if(UAC_RECODER.u16UAC_Buff_ReadIndex == UAC_RECODER.u16UAC_Buff_WriteIndex)
			{
				pu8buf[i*3+0] = 0x00;
				pu8buf[i*3+1] = 0x00;
				pu8buf[i*3+2] = 0x00;
			}
			else
			{
				if (++UAC_RECODER.u16UAC_Buff_ReadIndex >= DMIC_RING_BUFFER_LEN_4CH)       
					UAC_RECODER.u16UAC_Buff_ReadIndex = 0;
				
				pu8buf[i*3+0] = (g_au32PDMA2USB_RingBuff[UAC_RECODER.u16UAC_Buff_ReadIndex] >>  0) & 0xff;
				pu8buf[i*3+1] = (g_au32PDMA2USB_RingBuff[UAC_RECODER.u16UAC_Buff_ReadIndex] >>  8) & 0xff;
				pu8buf[i*3+2] = (g_au32PDMA2USB_RingBuff[UAC_RECODER.u16UAC_Buff_ReadIndex] >> 16) & 0xff;
			}
		}
		
		USBD_SET_PAYLOAD_LEN(EP8, u8EP8_Byte_Count);
	}
	else
	{	
		USBD_SET_PAYLOAD_LEN(EP8, 0);
	}
	
	return;
}

void EP3_Handler(void)
{
	
}

void EP4_Handler(void)
{
	
}

void EP5_Handler(void)
{
	
}

void AUDIO_Init(void)
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
    /* EP2 ==> Iso IN endpoint, address 2 */
    USBD_CONFIG_EP(EP2, USBD_CFG_EPMODE_IN | USBD_CFG_TYPE_ISO | EP2);
    /* Buffer range for EP2 */
    USBD_SET_EP_BUF_ADDR(EP2, EP2_BUF_BASE);

    /* EP6 ==> Iso IN endpoint, address 6 */
    USBD_CONFIG_EP(EP6, USBD_CFG_EPMODE_IN | USBD_CFG_TYPE_ISO | EP6);
    /* Buffer range for EP6 */
    USBD_SET_EP_BUF_ADDR(EP6, EP6_BUF_BASE);

    /* EP7 ==> Iso IN endpoint, address 7 */
    USBD_CONFIG_EP(EP7, USBD_CFG_EPMODE_IN | USBD_CFG_TYPE_ISO | EP7);
    /* Buffer range for EP7 */
    USBD_SET_EP_BUF_ADDR(EP7, EP7_BUF_BASE);

    /* EP8 ==> Iso IN endpoint, address 8 */
    USBD_CONFIG_EP(EP8, USBD_CFG_EPMODE_IN | USBD_CFG_TYPE_ISO | EP8);
    /* Buffer range for EP8 */
    USBD_SET_EP_BUF_ADDR(EP8, EP8_BUF_BASE);

}

void AUDIO_ClassRequest(void)
{
    uint8_t buf[8];
    uint32_t u32Temp;

    USBD_GetSetupPacket(buf);
    
    if((buf[0] & 0x7f) == 0x22) 	//bmRequestType= class, endpoint
	{ 
		if(buf[0] & 0x80)		// Device to host
		{
            switch(buf[1])
            {
                case UAC_GET_CUR:
                {
                    if(  buf[3]==SAMPLING_FREQ_CONTROL && (buf[4]==EP3))
					{
                      //CX  g_u8RecEn = 0;
                        u32Temp = UAC_PLAYBACK.usbd_PlaySampleRate;                                                          
                        M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 0) = (uint8_t)u32Temp;
                        M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = (uint8_t)u32Temp;
                        M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 2) = (uint8_t)u32Temp;
                        
                        /* Data stage */
                        USBD_SET_DATA1(EP0);
                        USBD_SET_PAYLOAD_LEN(EP0, 3);
                        printf("Get_CUR: %08X\n",UAC_PLAYBACK.usbd_PlaySampleRate); 

                    }
                    else
                    {
                        {
                            uint8_t i;
                            puts("error 1: ");
                            for(i=0;i<8;i++)
                                printf(" %2x", buf[i]);
                            
                            printf("\n");
                        }
                         /* error endpoint or un identified Control*/
                         /* Setup error, stall the device */
                         USBD_SetStall(0);
						 printf("Get_CUR USBD_SetStall(0): %08X\n",UAC_PLAYBACK.usbd_PlaySampleRate); 
                    }
                   
                    // Trigger next Control Out DATA1 Transaction.
                    /* Status stage */
                    USBD_PrepareCtrlOut(0, 0);
                break;
                }
								 
                default:
                {
                    /* Setup error, stall the device */
                    /*unidentify CONTROL*/
                    USBD_SetStall(0);
                }
                               
            }
            
            
        }
		else		// Host to device
		{
            switch(buf[1])
			{
                case UAC_SET_CUR:
                {   

                    if(buf[3]==SAMPLING_FREQ_CONTROL && (buf[4]==EP3))
					{    
                        USBD_PrepareCtrlOut((uint8_t *)&UAC_PLAYBACK.usbd_PlaySampleRate, buf[6]);
												                            
                        /* Status stage */
                        USBD_SET_DATA1(EP0);
                        USBD_SET_PAYLOAD_LEN(EP0, 0);
                           
                    }
					else
					{
                        {
                            uint8_t i;
                            puts("error 2: ");
                            for(i=0;i<8;i++)
                                printf(" %2x", buf[i]);
                            
                            printf("\n");
                        }
                        /* Setup error, stall the device */
                        /* Unidentify CONTROL*/
                        /* STALL control pipe */
                        USBD_SetStall(0);
                    }

                    break;
                }
                default:
                {
                    /*unimplement CONTROL or wrong endpoint number*/
                    /* Setup error, stall the device */
                    USBD_SetStall(0);
                }    
             }
        }
        
    }
	else		/*Feature unit control*/
	{ 
        if (buf[0] & 0x80)    // Device to host
        {
            
            switch(buf[1])
            {
                case UAC_GET_CUR:
                {
                    switch(buf[3])
                    {
                        case MUTE_CONTROL:
                        {
                            if(REC1CH_FEATURE_UNITID == buf[5])
                            {
                                u32Temp = UAC_RECODER.usbd_Rec1Mute;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                            }
                            else if(REC2CH_FEATURE_UNITID == buf[5])
                            {
                                u32Temp = UAC_RECODER.usbd_Rec2Mute;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                            }
//                             else if(REC3CH_FEATURE_UNITID == buf[5])
//                             {
//                                 u32Temp = UAC_RECODER.g_usbd_Rec3Mute;
//                                 M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
//                             }
                            else if(REC3CH_FEATURE_UNITID == buf[5])
                            {
                                u32Temp = UAC_RECODER.usbd_Rec3Mute;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                            }
                            else if(REC4CH_FEATURE_UNITID == buf[5])
                            {
                                u32Temp = UAC_RECODER.usbd_Rec4Mute;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                            }
                            else if(PLAY_FEATURE_UNITID == buf[5])
                            {
                              u32Temp = UAC_PLAYBACK.usbd_PlayMute;
                              M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                            }

                            /* Data stage */
                            USBD_SET_DATA1(EP0);
                            USBD_SET_PAYLOAD_LEN(EP0, 1);
                            break;
                        }
                        case VOLUME_CONTROL:
                        {
                                                
                            if(REC1CH_FEATURE_UNITID == buf[5])
                            {
                                /* Left or right channel */
                                if(buf[2] == 1)
                                {
                                    u32Temp = UAC_RECODER.usbd_Rec1VolumeL;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                    u32Temp = UAC_RECODER.usbd_Rec1VolumeL >> 8;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                                }
                                else
                                {
                                    u32Temp = UAC_RECODER.usbd_Rec1VolumeR;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                    u32Temp = UAC_RECODER.usbd_Rec1VolumeR >> 8;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                                }

                            }
                            else if(REC2CH_FEATURE_UNITID == buf[5])
                            {
                                /* Left or right channel */
                                if(buf[2] == 1)
                                {
                                    u32Temp = UAC_RECODER.usbd_Rec2VolumeL;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                    u32Temp = UAC_RECODER.usbd_Rec2VolumeL >> 8;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                                }
                                else
                                {
                                    u32Temp = UAC_RECODER.usbd_Rec2VolumeR;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                    u32Temp = UAC_RECODER.usbd_Rec2VolumeR >> 8;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                                }

                            }
                            else if(REC3CH_FEATURE_UNITID == buf[5])
                            {
                                /* Left or right channel */
                                if(buf[2] == 1)
                                {
                                    u32Temp = UAC_RECODER.usbd_Rec3VolumeL;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                    u32Temp = UAC_RECODER.usbd_Rec3VolumeL >> 8;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                                }
                                else
                                {
                                    u32Temp = UAC_RECODER.usbd_Rec3VolumeR;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                    u32Temp = UAC_RECODER.usbd_Rec3VolumeR >> 8;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                                }

                            }
                            else if(REC4CH_FEATURE_UNITID == buf[5])
                            {
                                /* Left or right channel */
                                if(buf[2] == 1)
                                {
                                    u32Temp = UAC_RECODER.usbd_Rec4VolumeL;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                    u32Temp = UAC_RECODER.usbd_Rec4VolumeL >> 8;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                                }
                                else
                                {
                                    u32Temp = UAC_RECODER.usbd_Rec4VolumeR;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                    u32Temp = UAC_RECODER.usbd_Rec4VolumeR >> 8;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                                }

                            }
                            else if(PLAY_FEATURE_UNITID == buf[5])
                            {
                                /* Left or right channel */
                                if(buf[2] == 1)
                                {
                                  u32Temp = UAC_PLAYBACK.usbd_PlayVolumeL;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                    u32Temp = UAC_PLAYBACK.usbd_PlayVolumeL >> 8;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                                }
                                else
                                {
                                  u32Temp = UAC_PLAYBACK.usbd_PlayVolumeR;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                    u32Temp = UAC_PLAYBACK.usbd_PlayVolumeR >> 8;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                                }
                            }

                            /* Data stage */
                            USBD_SET_DATA1(EP0);
                            USBD_SET_PAYLOAD_LEN(EP0, 2);
                            break;
                        }
                        default:
                        {
                            /* Setup error, stall the device */
                            USBD_SetStall(0);
                        }
                    }

                    // Trigger next Control Out DATA1 Transaction.
                    /* Status stage */
                    USBD_PrepareCtrlOut(0, 0);
                    break;
                }

                case UAC_GET_MIN:
                {
                    switch(buf[3])
                    {
                        case VOLUME_CONTROL:
                        {
                            if(REC1CH_FEATURE_UNITID == buf[5])
                            {
                                u32Temp = UAC_RECODER.usbd_Rec1MinVolume;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                u32Temp = UAC_RECODER.usbd_Rec1MinVolume >> 8;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                            }
                            else if(REC2CH_FEATURE_UNITID == buf[5])
                            {
                                u32Temp = UAC_RECODER.usbd_Rec2MinVolume;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                u32Temp = UAC_RECODER.usbd_Rec2MinVolume >> 8;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                            }
                            else if(REC3CH_FEATURE_UNITID == buf[5])
                            {
                                u32Temp = UAC_RECODER.usbd_Rec3MinVolume;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                u32Temp = UAC_RECODER.usbd_Rec3MinVolume >> 8;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                            }
                            else if(REC4CH_FEATURE_UNITID == buf[5])
                            {
                                u32Temp = UAC_RECODER.usbd_Rec4MinVolume;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                u32Temp = UAC_RECODER.usbd_Rec4MinVolume >> 8;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                            }
                            else if(PLAY_FEATURE_UNITID == buf[5])
                            {
                              u32Temp = UAC_PLAYBACK.usbd_PlayMinVolume;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                u32Temp = UAC_PLAYBACK.usbd_PlayMinVolume >> 8;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                            }
                            /* Data stage */
                            USBD_SET_DATA1(EP0);
                            USBD_SET_PAYLOAD_LEN(EP0, 2);
                            break;
                        }
                        default:
                            /* STALL control pipe */
                            USBD_SetStall(0);
                    }
                    // Trigger next Control Out DATA1 Transaction.
                    /* Status stage */
                    USBD_PrepareCtrlOut(0, 0);
                    break;
                }

                case UAC_GET_MAX:
                {
                    switch(buf[3])
                    {
                        case VOLUME_CONTROL:
                        {
                            if(REC1CH_FEATURE_UNITID == buf[5])
                            {
                                u32Temp = UAC_RECODER.usbd_Rec1MaxVolume;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                u32Temp = UAC_RECODER.usbd_Rec1MaxVolume >> 8;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                            }
                            else if(REC2CH_FEATURE_UNITID == buf[5])
                            {
                                u32Temp = UAC_RECODER.usbd_Rec2MaxVolume;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                u32Temp = UAC_RECODER.usbd_Rec2MaxVolume >> 8;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                            }
                            else if(REC3CH_FEATURE_UNITID == buf[5])
                            {
                                u32Temp = UAC_RECODER.usbd_Rec3MaxVolume;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                u32Temp = UAC_RECODER.usbd_Rec3MaxVolume >> 8;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                            }
                            else if(REC4CH_FEATURE_UNITID == buf[5])
                            {
                                u32Temp = UAC_RECODER.usbd_Rec4MaxVolume;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                u32Temp = UAC_RECODER.usbd_Rec4MaxVolume >> 8;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                            }
                            else if(PLAY_FEATURE_UNITID == buf[5])
                            {
                              u32Temp = UAC_PLAYBACK.usbd_PlayMaxVolume;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                u32Temp = UAC_PLAYBACK.usbd_PlayMaxVolume >> 8;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                            }
                            /* Data stage */
                            USBD_SET_DATA1(EP0);
                            USBD_SET_PAYLOAD_LEN(EP0, 2);
                            break;
                        }
                        default:
                            
                        /* STALL control pipe */
                        USBD_SetStall(0);
                    }
										
                    // Trigger next Control Out DATA1 Transaction.
                    /* Status stage */
                    USBD_PrepareCtrlOut(0, 0);
                    break;
                }

                case UAC_GET_RES:
                {
                    switch(buf[3])
                    {
                        case VOLUME_CONTROL:
                        {
                            if(REC1CH_FEATURE_UNITID == buf[5])
                            {
                                u32Temp = UAC_RECODER.usbd_Rec1ResVolume;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                u32Temp = UAC_RECODER.usbd_Rec1ResVolume >> 8;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                            }
                            else if(REC2CH_FEATURE_UNITID == buf[5])
                            {
                                u32Temp = UAC_RECODER.usbd_Rec2ResVolume;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                u32Temp = UAC_RECODER.usbd_Rec2ResVolume >> 8;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                            }
                            else if(REC3CH_FEATURE_UNITID == buf[5])
                            {
                                u32Temp = UAC_RECODER.usbd_Rec3ResVolume;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                u32Temp = UAC_RECODER.usbd_Rec3ResVolume >> 8;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                            }
                            else if(REC4CH_FEATURE_UNITID == buf[5])
                            {
                                u32Temp = UAC_RECODER.usbd_Rec4ResVolume;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                u32Temp = UAC_RECODER.usbd_Rec4ResVolume >> 8;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = u32Temp;
                            }
                            else if(PLAY_FEATURE_UNITID == buf[5])
                            {
                              u32Temp = UAC_PLAYBACK.usbd_PlayResVolume;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = u32Temp;
                                u32Temp = UAC_PLAYBACK.usbd_PlayResVolume >> 8;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) =u32Temp;
                            }
                            /* Data stage */
                            USBD_SET_DATA1(EP0);
                            USBD_SET_PAYLOAD_LEN(EP0, 2);
                            break;
                        }
                        default:
                        /* STALL control pipe */
                        USBD_SetStall(0);
                    }
                    // Trigger next Control Out DATA1 Transaction.
                    /* Status stage */
                    USBD_PrepareCtrlOut(0, 0);
                    break;
                }
								

                default:
                {
                    /* Setup error, stall the device */
                    USBD_SetStall(0);
                }
            }
        }
        else
        {
            // Host to device
            switch(buf[1])
            {
                case UAC_SET_CUR:
                {
                    switch(buf[3])
                    {
                        case MUTE_CONTROL:
                            if(REC1CH_FEATURE_UNITID == buf[5])
                                USBD_PrepareCtrlOut((uint8_t *)&UAC_RECODER.usbd_Rec1Mute, buf[6]);
                            else if(REC2CH_FEATURE_UNITID == buf[5])
                                USBD_PrepareCtrlOut((uint8_t *)&UAC_RECODER.usbd_Rec2Mute, buf[6]);
                            else if(REC3CH_FEATURE_UNITID == buf[5])
                                USBD_PrepareCtrlOut((uint8_t *)&UAC_RECODER.usbd_Rec3Mute, buf[6]);
                            else if(REC4CH_FEATURE_UNITID == buf[5])
                                USBD_PrepareCtrlOut((uint8_t *)&UAC_RECODER.usbd_Rec4Mute, buf[6]);
                            else if(PLAY_FEATURE_UNITID == buf[5])
                            {
                                USBD_PrepareCtrlOut((uint8_t *)&UAC_PLAYBACK.usbd_PlayMute, buf[6]);
                            }
                            /* Status stage */
                            USBD_SET_DATA1(EP0);
                            USBD_SET_PAYLOAD_LEN(EP0, 0);
                            break;

                        case VOLUME_CONTROL:
                            if(REC1CH_FEATURE_UNITID == buf[5])
                            {
                                if(buf[2] == 1)
                                {
                                    /* Prepare the buffer for new record volume of left channel */
                                    USBD_PrepareCtrlOut((uint8_t *)&UAC_RECODER.usbd_Rec1VolumeL, buf[6]);
                                }
                                else
                                {
                                    /* Prepare the buffer for new record volume of right channel */
                                    USBD_PrepareCtrlOut((uint8_t *)&UAC_RECODER.usbd_Rec1VolumeR, buf[6]);
                                }
                            }
                            else if(REC2CH_FEATURE_UNITID == buf[5])
                            {
                                if(buf[2] == 1)
                                {
                                    /* Prepare the buffer for new record volume of left channel */
                                    USBD_PrepareCtrlOut((uint8_t *)&UAC_RECODER.usbd_Rec2VolumeL, buf[6]);
                                }
                                else
                                {
                                    /* Prepare the buffer for new record volume of right channel */
                                    USBD_PrepareCtrlOut((uint8_t *)&UAC_RECODER.usbd_Rec2VolumeR, buf[6]);
                                }
                            }
                            else if(REC3CH_FEATURE_UNITID == buf[5])
                            {
                                if(buf[2] == 1)
                                {
                                    /* Prepare the buffer for new record volume of left channel */
                                    USBD_PrepareCtrlOut((uint8_t *)&UAC_RECODER.usbd_Rec3VolumeL, buf[6]);
                                }
                                else
                                {
                                    /* Prepare the buffer for new record volume of right channel */
                                    USBD_PrepareCtrlOut((uint8_t *)&UAC_RECODER.usbd_Rec3VolumeR, buf[6]);
                                }
                            }
                            else if(REC4CH_FEATURE_UNITID == buf[5])
                            {
                                if(buf[2] == 1)
                                {
                                    /* Prepare the buffer for new record volume of left channel */
                                    USBD_PrepareCtrlOut((uint8_t *)&UAC_RECODER.usbd_Rec4VolumeL, buf[6]);
                                }
                                else
                                {
                                    /* Prepare the buffer for new record volume of right channel */
                                    USBD_PrepareCtrlOut((uint8_t *)&UAC_RECODER.usbd_Rec4VolumeR, buf[6]);
                                }
                            }
                            else if(PLAY_FEATURE_UNITID == buf[5])
                            {

                                if(buf[2] == 1)
                                {
                                    /* Prepare the buffer for new play volume of left channel */
                                    USBD_PrepareCtrlOut((uint8_t *)&UAC_PLAYBACK.usbd_PlayVolumeL, buf[6]);
                                }
                                else
                                {
                                    /* Prepare the buffer for new play volume of right channel */
                                    USBD_PrepareCtrlOut((uint8_t *)&UAC_PLAYBACK.usbd_PlayVolumeR, buf[6]);
                                }
                            }

                            /* Status stage */
                            USBD_SET_DATA1(EP0);
                            USBD_SET_PAYLOAD_LEN(EP0, 0);
                            break;

                        default:
                            /* STALL control pipe */
                            USBD_SetStall(0);
                            break;
                    }
                    break;
                }
								// HID 
                case HID_SET_REPORT:
                {
                    if(buf[3] == 2)
                    {
                        /* Request Type = Output */
                        USBD_SET_DATA1(EP1);
                        USBD_SET_PAYLOAD_LEN(EP1, buf[6]);

                        /* Status stage */
                        USBD_PrepareCtrlIn(0, 0);
                    }
                    break;
                }
                case HID_SET_IDLE:
                {
                    /* Status stage */
                    USBD_SET_DATA1(EP0);
                    USBD_SET_PAYLOAD_LEN(EP0, 0);
                    break;
                }
                case HID_SET_PROTOCOL:

                default:
                {
                    /* Setup error, stall the device */
                    USBD_SetStall(0);
                    break;
                }
            }
        }
    }
}

/**
 * @brief       Set Interface standard request
 *
 * @param[in]   u32AltInterface Interface
 *
 * @return      None
 *
 * @details     This function is used to set UAC Class relative setting
 */
void AUDIO_SetInterface(void)			
{
	uint8_t buf[8];
	uint8_t u32AltInterface;
	uint8_t g_usbd_UsbInterface;
	volatile int i;

	USBD_GetSetupPacket(buf);

	u32AltInterface = buf[2];
	g_usbd_UsbInterface = buf[4];
	
	if ( g_usbd_UsbInterface == REC_IFNUM_1CH )		 
	{
		if (u32AltInterface == 1)
		{
			UAC_RECODER.u8AudioRecInterfConfig = REC_IFNUM_1CH;
			UAC_RECODER.u8AudioMicState = UAC_START_AUDIO_RECORD;
				
			USBD_SET_DATA1(EP2);
			USBD_SET_PAYLOAD_LEN(EP2, 0);						
			UAC_RECODER.u8DMIC_Buff_Index = 1;
            
			PDMA_DISABLE_CHANNEL(PDMA_CH2_MASK);
			PDMA_CONFIG.play_en = 0;
			PDMA_CONFIG.rec_en  = 1;
			PDMA_CONFIG.rec_buflen = REC_RATE*REC_CHANNELNUM_1CH/1000;
			PDMA_Init(&PDMA_CONFIG);
			DMIC_Init(REC_RATE, 1);
		}
		else
		{
			UAC_RECODER.u8AudioMicState = UAC_STOP_AUDIO_RECORD;
			UAC_RECODER.u16UAC_Buff_ReadIndex = 0;
			UAC_RECODER.u16UAC_Buff_WriteIndex = 0;
			USBD_SET_DATA1(EP2);
			USBD_SET_PAYLOAD_LEN(EP2, 0);
			DMIC_Close(DMIC);
		}
	}
    else if ( g_usbd_UsbInterface == REC_IFNUM_2CH )		 
	{
		if (u32AltInterface == 1)
		{
			UAC_RECODER.u8AudioRecInterfConfig = REC_IFNUM_2CH;
            
			UAC_RECODER.u8AudioMicState = UAC_START_AUDIO_RECORD;
				
			USBD_SET_DATA1(EP6);
			USBD_SET_PAYLOAD_LEN(EP6, 0);						
			UAC_RECODER.u8DMIC_Buff_Index = 1;
            
			PDMA_DISABLE_CHANNEL(PDMA_CH2_MASK);
			PDMA_CONFIG.play_en = 0;
			PDMA_CONFIG.rec_en  = 1;
			PDMA_CONFIG.rec_buflen = REC_RATE*REC_CHANNELNUM_2CH/1000;
			PDMA_Init(&PDMA_CONFIG);
			DMIC_Init(REC_RATE, 2);
		}
		else
		{          
			UAC_RECODER.u8AudioMicState = UAC_STOP_AUDIO_RECORD;
			UAC_RECODER.u16UAC_Buff_ReadIndex = 0;
			UAC_RECODER.u16UAC_Buff_WriteIndex = 0;   
			USBD_SET_DATA1(EP6);
			USBD_SET_PAYLOAD_LEN(EP6, 0);
			DMIC_Close(DMIC);
		}
	}
    else if ( g_usbd_UsbInterface == REC_IFNUM_3CH )		 
	{
		if (u32AltInterface == 1)
		{
			UAC_RECODER.u8AudioRecInterfConfig = REC_IFNUM_3CH;
			
			UAC_RECODER.u8AudioMicState = UAC_START_AUDIO_RECORD;
				
			USBD_SET_DATA1(EP7);
			USBD_SET_PAYLOAD_LEN(EP7, 0);						
			UAC_RECODER.u8DMIC_Buff_Index = 1;
            
			PDMA_DISABLE_CHANNEL(PDMA_CH2_MASK);
			PDMA_CONFIG.play_en = 0;
			PDMA_CONFIG.rec_en  = 1;
			PDMA_CONFIG.rec_buflen = REC_RATE*REC_CHANNELNUM_3CH/1000;
			PDMA_Init(&PDMA_CONFIG);
			DMIC_Init(REC_RATE, 3);
		}
		else
		{
            
			UAC_RECODER.u8AudioMicState = UAC_STOP_AUDIO_RECORD;
			UAC_RECODER.u16UAC_Buff_ReadIndex = 0;
			UAC_RECODER.u16UAC_Buff_WriteIndex = 0;
			USBD_SET_DATA1(EP7);
			USBD_SET_PAYLOAD_LEN(EP7, 0);
			DMIC_Close(DMIC);
		}
	}
    else if ( g_usbd_UsbInterface == REC_IFNUM_4CH )		 
	{
		if (u32AltInterface == 1)
		{
			UAC_RECODER.u8AudioRecInterfConfig = REC_IFNUM_4CH;
            
			UAC_RECODER.u8AudioMicState = UAC_START_AUDIO_RECORD;
				
			USBD_SET_DATA1(EP8);
			USBD_SET_PAYLOAD_LEN(EP8, 0);						
			UAC_RECODER.u8DMIC_Buff_Index = 1;
            
			PDMA_DISABLE_CHANNEL(PDMA_CH2_MASK);
			PDMA_CONFIG.play_en = 0;
			PDMA_CONFIG.rec_en  = 1;
			PDMA_CONFIG.rec_buflen = REC_RATE*REC_CHANNELNUM_4CH/1000;
			PDMA_Init(&PDMA_CONFIG);
			DMIC_Init(REC_RATE, 4);
		}
		else
		{
			UAC_RECODER.u8AudioMicState = UAC_STOP_AUDIO_RECORD;
			UAC_RECODER.u16UAC_Buff_ReadIndex = 0;
			UAC_RECODER.u16UAC_Buff_WriteIndex = 0;
			USBD_SET_DATA1(EP8);
			USBD_SET_PAYLOAD_LEN(EP8, 0);
			DMIC_Close(DMIC);
		}
	}
}

void AUDIO_SetConfig(void)
{
    // Clear stall status and ready
    USBD->EP[2].CFGP = 1;
    USBD->EP[3].CFGP = 1;
    /*****************************************************/
    /* EP2 ==> Bulk IN endpoint, address 2 */
    USBD_CONFIG_EP(EP2, USBD_CFG_EPMODE_IN | EP2);
    /* Buffer range for EP2 */
    USBD_SET_EP_BUF_ADDR(EP2, EP2_BUF_BASE);

    /* EP3 ==> Bulk Out endpoint, address 3 */
    USBD_CONFIG_EP(EP3, USBD_CFG_EPMODE_OUT | EP3);
    /* Buffer range for EP3 */
    USBD_SET_EP_BUF_ADDR(EP3, EP3_BUF_BASE);

    /* trigger to receive OUT data */
    USBD_SET_PAYLOAD_LEN(EP3, EP3_MAX_PKT_SIZE);

    USBD_LockEpStall(0);

    DBG_PRINTF("Set config\n");

}

void DataFlashRead(uint32_t addr, uint32_t size, uint32_t buffer)
{
    //DataFlashRead(addr, size, (uint32_t)buffer);
    USBD_MemCopy((uint8_t *)buffer, (uint8_t *)(addr + 0x20004000), size);
}


void DataFlashWrite(uint32_t addr, uint32_t size, uint32_t buffer)
{
    USBD_MemCopy((uint8_t *)(addr + 0x20004000), (uint8_t *)buffer, size);
}
