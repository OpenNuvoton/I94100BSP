/******************************************************************************
 * @file     MassStorage.c
 * @brief    USBD driver Sample file
 *
 * @note
 * Copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/

/*!<Includes */
#include <stdio.h>
#include <string.h>
#include "Platform.h"
#include "usbd_audio.h"
#include "audioclass.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Extern functions declaration                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
extern void MIC_Start(void);
extern void MIC_Stop(void);

/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables and constants                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
extern void I2S_PDMA_Init(uint8_t ChannelNum);
static uint32_t g_u32SpeakerByteCount, g_u32SpeakerByteCountRec;
static uint8_t g_u8AudioSpeakerDebounce;
static uint16_t g_u16StartOfFrameCount;

#if 0
	#define DBG_PRINTF      printf
#else
	#define DBG_PRINTF(...)
#endif

typedef __packed union {
	__packed struct
	{
		uint8_t Recipient : 5;
		uint8_t Type      : 2;     
		uint8_t Dir       : 1;		
	} BM;
	
	uint8_t B;	
} REQUEST_TYPE;
	
typedef __packed union {
	__packed struct 
	{
		uint8_t L;
		uint8_t H;
	} WB;
	
	uint16_t W;	
} DB16;

typedef __packed struct {
  //uint8_t     bmRequestType;
	REQUEST_TYPE 	bmRequestType;
	uint8_t   		bRequest;
	DB16  			wValue;
	DB16  			wIndex;
	DB16  			wLength;
} SetupPkt_t;

//Get MAX or MIN value
#define GetMax( x1, x2 ) ( ( x1 ) > ( x2 ) ? ( x1 ) : ( x2 ) )
#define GetMin( x1, x2 ) ( ( x1 ) > ( x2 ) ? ( x2 ) : ( x1 ) )

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
			printf("PI\n");
			PB->PUSEL |= BIT31;
			// USB Plug In 
			USBD_ENABLE_USB();
		} 
		else 
		{
			printf("PO\n");
			PB->PUSEL &= ~(BIT30|BIT31);
			// USB Un-plug 
			USBD_DISABLE_USB();
		}
	}
	
	if ( u32IntSts & USBD_INTSTS_SOFIF_Msk )
	{
        if (g_u16StartOfFrameCount++ == 3000)
        {
            printf("a");
            g_u16StartOfFrameCount = 0;
            SYS->IRCTCTL = 0x503;
        }
        
        // Clear event flag 
		USBD_CLR_INT_FLAG(USBD_INTSTS_SOFIF_Msk);
		if ( g_u32SpeakerByteCountRec != g_u32SpeakerByteCount )
		{
			g_u32SpeakerByteCountRec = g_u32SpeakerByteCount;
		}
		else
		{
			g_u8AudioSpeakerDebounce++;
			
			if (g_u8AudioSpeakerDebounce >= 5)
			{
				g_u8AudioSpeakerDebounce = 0;
				g_u32SpeakerByteCount = 0; 
				g_u32SpeakerByteCountRec = 0;
			}
		}
	}	
	
	if(u32IntSts & USBD_INTSTS_BUS) 
	{
		// Clear event flag 
		USBD_CLR_INT_FLAG(USBD_INTSTS_BUS);

		if(u32State & USBD_STATE_USBRST) 
		{
			// Bus reset 
			USBD_ENABLE_USB();
			USBD_SwReset();
			DBG_PRINTF("Bus reset\n");
		}
		if(u32State & USBD_STATE_SUSPEND) 
		{
			// Enable USB but disable PHY 
			USBD_DISABLE_PHY();
			DBG_PRINTF("Suspend\n");
		}
		if(u32State & USBD_STATE_RESUME) 
		{
			// Enable USB and enable PHY 
			USBD_ENABLE_USB();
			DBG_PRINTF("Resume\n");
		}
	}

	if(u32IntSts & USBD_INTSTS_USB)
	{
		// EP events
		if(u32IntSts & USBD_INTSTS_EP0)
		{
			// Clear event flag 
			USBD_CLR_INT_FLAG(USBD_INTSTS_EP0);
			// control IN
			USBD_CtrlIn();
		}
		if(u32IntSts & USBD_INTSTS_EP1) 
		{
			// Clear event flag 
			USBD_CLR_INT_FLAG(USBD_INTSTS_EP1);
			// control OUT
			USBD_CtrlOut();
		}
		if(u32IntSts & USBD_INTSTS_EP2) 
		{
			// Clear event flag 
			USBD_CLR_INT_FLAG(USBD_INTSTS_EP2);
			EP2_Handler();
		}
		if(u32IntSts & USBD_INTSTS_EP3) 
		{
			// Clear event flag 
			USBD_CLR_INT_FLAG(USBD_INTSTS_EP3);
			EP3_Handler();
		}
		if(u32IntSts & USBD_INTSTS_EP4) 
		{
			// Clear event flag 
			USBD_CLR_INT_FLAG(USBD_INTSTS_EP4);
			EP4_Handler();
		}
		if(u32IntSts & USBD_INTSTS_EP5) 
		{
			// Clear event flag 
			USBD_CLR_INT_FLAG(USBD_INTSTS_EP5);
            EP5_Handler();
		}
		if(u32IntSts & USBD_INTSTS_EP6) 
		{
			// Clear event flag 
			USBD_CLR_INT_FLAG(USBD_INTSTS_EP6);
		}
		if(u32IntSts & USBD_INTSTS_EP7)
		{
			// Clear event flag 
			USBD_CLR_INT_FLAG(USBD_INTSTS_EP7);
		}
		// USB event
		if(u32IntSts & USBD_INTSTS_SETUP)
		{
			// Setup packet
			// Clear event flag 
			USBD_CLR_INT_FLAG(USBD_INTSTS_SETUP);

			// Clear the data IN/OUT ready flag of control end-points 
			USBD_STOP_TRANSACTION(EP0);
			USBD_STOP_TRANSACTION(EP1);

			USBD_ProcessSetupPacket();
		}
	}
}


void EP2_Handler(void)
{	
	int i;
	uint8_t u8EP2_Byte_Count;
	uint8_t *pu8buf;
    uint16_t  u16Distance;
    
    if ( UAC_REC.g_u8Current_Mic_ChannNum != UAC_REC_CHANNEL_1CH)
    {
        USBD_SET_PAYLOAD_LEN(EP2, 0);
        return;
    }
    
	// UAC record in busy.
	if (UAC_REC.g_usbd_UsbAudioState == UAC_BUSY_AUDIO_RECORD)
	{		
		// This section calculate the data number in UAC_RingBuffer.
		// And set the next transfer size depends on buffer threshold.
		if ( UAC_REC.g_u16UAC_Buff_WriteIndex >= UAC_REC.g_u16UAC_Buff_ReadIndex ) {
			u16Distance = UAC_REC.g_u16UAC_Buff_WriteIndex - UAC_REC.g_u16UAC_Buff_ReadIndex;	
		}
		else {
			u16Distance = (AMIC_RING_BUFFER_LEN_12CH - UAC_REC.g_u16UAC_Buff_ReadIndex) + UAC_REC.g_u16UAC_Buff_WriteIndex;	
		}
		
		if ( u16Distance > AMIC_BUFF_UPPER_THRE_12CH ) {
			u8EP2_Byte_Count = EP2BC_BASE + AMIC_EP2BC_OFFSET;		
		}
		else if ( u16Distance < AMIC_BUFF_LOWER_THRE_12CH ) {
			u8EP2_Byte_Count = EP2BC_BASE - AMIC_EP2BC_OFFSET;		
		}
		else {
			u8EP2_Byte_Count = EP2BC_BASE;
		}

		pu8buf = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP2));		
		        
        for ( i= 0 ; i < (u8EP2_Byte_Count/2) ; i++) 
        {
            pu8buf[i*2+0] = (UAC_REC.g_au32UAC_RingBuff[UAC_REC.g_u16UAC_Buff_ReadIndex] >>  0) & 0xff;		
            pu8buf[i*2+1] = (UAC_REC.g_au32UAC_RingBuff[UAC_REC.g_u16UAC_Buff_ReadIndex] >>  8) & 0xff;
            UAC_REC.g_u16UAC_Buff_ReadIndex++;	

            if ( UAC_REC.g_u16UAC_Buff_ReadIndex == AMIC_RING_BUFFER_LEN_12CH ) 
                UAC_REC.g_u16UAC_Buff_ReadIndex = 0;
        }	
           
		USBD_SET_PAYLOAD_LEN(EP2, u8EP2_Byte_Count);		
        
	}
	else
	{	
		USBD_SET_PAYLOAD_LEN(EP2, 0);
	}

		
}

void EP3_Handler(void)
{
    int i;
	uint8_t u8EP3_Byte_Count;
	uint8_t *pu8buf;
    uint16_t  u16Distance;
    
	if ( UAC_REC.g_u8Current_Mic_ChannNum != UAC_REC_CHANNEL_2CH)
    {
        USBD_SET_PAYLOAD_LEN(EP3, 0);
        return;
    }
    
    // UAC record in busy.
	if (UAC_REC.g_usbd_UsbAudioState == UAC_BUSY_AUDIO_RECORD)
	{		
		// This section calculate the data number in UAC_RingBuffer.
		// And set the next transfer size depends on buffer threshold.
		if ( UAC_REC.g_u16UAC_Buff_WriteIndex >= UAC_REC.g_u16UAC_Buff_ReadIndex ) {
			u16Distance = UAC_REC.g_u16UAC_Buff_WriteIndex - UAC_REC.g_u16UAC_Buff_ReadIndex;	
		}
		else {
			u16Distance = (AMIC_RING_BUFFER_LEN_12CH - UAC_REC.g_u16UAC_Buff_ReadIndex) + UAC_REC.g_u16UAC_Buff_WriteIndex;	
		}
		
		if ( u16Distance > AMIC_BUFF_UPPER_THRE_12CH ) {
			u8EP3_Byte_Count = EP3BC_BASE + AMIC_EP3BC_OFFSET;		
		}
		else if ( u16Distance < AMIC_BUFF_LOWER_THRE_12CH ) {
			u8EP3_Byte_Count = EP3BC_BASE - AMIC_EP3BC_OFFSET;		
		}
		else {
			u8EP3_Byte_Count = EP3BC_BASE;
		}

		pu8buf = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP3));				
        for ( i= 0 ; i < (u8EP3_Byte_Count/4) ; i++)
        {
            pu8buf[i*4+0] = (UAC_REC.g_au32UAC_RingBuff[UAC_REC.g_u16UAC_Buff_ReadIndex] >>  0) & 0xff;		
            pu8buf[i*4+1] = (UAC_REC.g_au32UAC_RingBuff[UAC_REC.g_u16UAC_Buff_ReadIndex] >>  8) & 0xff;		

            pu8buf[i*4+2] = (UAC_REC.g_au32UAC_RingBuff[UAC_REC.g_u16UAC_Buff_ReadIndex] >> 16) & 0xff;		
            pu8buf[i*4+3] = (UAC_REC.g_au32UAC_RingBuff[UAC_REC.g_u16UAC_Buff_ReadIndex] >> 24) & 0xff;		
            UAC_REC.g_u16UAC_Buff_ReadIndex++;	

            if ( UAC_REC.g_u16UAC_Buff_ReadIndex == AMIC_RING_BUFFER_LEN_12CH ) 
                UAC_REC.g_u16UAC_Buff_ReadIndex = 0;
        }
              
		USBD_SET_PAYLOAD_LEN(EP3, u8EP3_Byte_Count);	
       
	}
	else
	{	
		USBD_SET_PAYLOAD_LEN(EP3, 0);
	}
    
}

void EP4_Handler(void)
{
    int i;
	uint8_t u8EP4_Byte_Count;
	uint8_t *pu8buf;
    uint16_t  u16Distance;
    
	if ( UAC_REC.g_u8Current_Mic_ChannNum != UAC_REC_CHANNEL_3CH)
    {
        USBD_SET_PAYLOAD_LEN(EP4, 0);
        return;
    }
    
    // UAC record in busy.
	if (UAC_REC.g_usbd_UsbAudioState == UAC_BUSY_AUDIO_RECORD)
	{		
		// This section calculate the data number in UAC_RingBuffer.
		// And set the next transfer size depends on buffer threshold.
		if ( UAC_REC.g_u16UAC_Buff_WriteIndex >= UAC_REC.g_u16UAC_Buff_ReadIndex ) {
			u16Distance = UAC_REC.g_u16UAC_Buff_WriteIndex - UAC_REC.g_u16UAC_Buff_ReadIndex;	
		}
		else {
			u16Distance = (AMIC_RING_BUFFER_LEN_34CH - UAC_REC.g_u16UAC_Buff_ReadIndex) + UAC_REC.g_u16UAC_Buff_WriteIndex;	
		}
		
		if ( u16Distance > AMIC_BUFF_UPPER_THRE_34CH ) {
			u8EP4_Byte_Count = EP4BC_BASE + AMIC_EP4BC_OFFSET;		
		}
		else if ( u16Distance < AMIC_BUFF_LOWER_THRE_34CH ) {
			u8EP4_Byte_Count = EP4BC_BASE - AMIC_EP4BC_OFFSET;		
		}
		else {
			u8EP4_Byte_Count = EP4BC_BASE;
		}

		pu8buf = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP4));		
		
        
        for ( i= 0 ; i < (u8EP4_Byte_Count/6) ; i++)
        {   
            pu8buf[i*6+0] = (UAC_REC.g_au32UAC_RingBuff[UAC_REC.g_u16UAC_Buff_ReadIndex] >>  0) & 0xff;		
            pu8buf[i*6+1] = (UAC_REC.g_au32UAC_RingBuff[UAC_REC.g_u16UAC_Buff_ReadIndex] >>  8) & 0xff;		

            pu8buf[i*6+2] = (UAC_REC.g_au32UAC_RingBuff[UAC_REC.g_u16UAC_Buff_ReadIndex] >>  16) & 0xff;	
            pu8buf[i*6+3] = (UAC_REC.g_au32UAC_RingBuff[UAC_REC.g_u16UAC_Buff_ReadIndex] >>  24) & 0xff;		
            UAC_REC.g_u16UAC_Buff_ReadIndex++;	
            
            if ( UAC_REC.g_u16UAC_Buff_ReadIndex == AMIC_RING_BUFFER_LEN_34CH ) 
                UAC_REC.g_u16UAC_Buff_ReadIndex = 0;
            
            pu8buf[i*6+4] = (UAC_REC.g_au32UAC_RingBuff[UAC_REC.g_u16UAC_Buff_ReadIndex] >>  0) & 0xff;		
            pu8buf[i*6+5] = (UAC_REC.g_au32UAC_RingBuff[UAC_REC.g_u16UAC_Buff_ReadIndex] >>  8) & 0xff;		
            UAC_REC.g_u16UAC_Buff_ReadIndex++;	

            if ( UAC_REC.g_u16UAC_Buff_ReadIndex == AMIC_RING_BUFFER_LEN_34CH ) 
                UAC_REC.g_u16UAC_Buff_ReadIndex = 0;
            
        }	
           
        
		USBD_SET_PAYLOAD_LEN(EP4, u8EP4_Byte_Count);	
           
	}
	else
	{	
		USBD_SET_PAYLOAD_LEN(EP4, 0);
	}
    
}

void EP5_Handler(void)
{
    int i;
	uint8_t u8EP5_Byte_Count;
	uint8_t *pu8buf;
    uint16_t  u16Distance;
    
	if ( UAC_REC.g_u8Current_Mic_ChannNum != UAC_REC_CHANNEL_4CH)
    {
        USBD_SET_PAYLOAD_LEN(EP5, 0);
        return;
    }
    
    // UAC record in busy.
	if (UAC_REC.g_usbd_UsbAudioState == UAC_BUSY_AUDIO_RECORD)
	{		
		// This section calculate the data number in UAC_RingBuffer.
		// And set the next transfer size depends on buffer threshold.
		if ( UAC_REC.g_u16UAC_Buff_WriteIndex >= UAC_REC.g_u16UAC_Buff_ReadIndex ) {
			u16Distance = UAC_REC.g_u16UAC_Buff_WriteIndex - UAC_REC.g_u16UAC_Buff_ReadIndex;	
		}
		else {
			u16Distance = (AMIC_RING_BUFFER_LEN_34CH - UAC_REC.g_u16UAC_Buff_ReadIndex) + UAC_REC.g_u16UAC_Buff_WriteIndex;	
		}
		
		if ( u16Distance > AMIC_BUFF_UPPER_THRE_34CH ) {
			u8EP5_Byte_Count = EP5BC_BASE + AMIC_EP5BC_OFFSET;		
		}
		else if ( u16Distance < AMIC_BUFF_LOWER_THRE_34CH ) {
			u8EP5_Byte_Count = EP5BC_BASE - AMIC_EP5BC_OFFSET;		
		}
		else {
			u8EP5_Byte_Count = EP5BC_BASE;
		}

		pu8buf = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP5));		        
        for ( i= 0 ; i < (u8EP5_Byte_Count/4) ; i++)
        {
            pu8buf[i*4+0] = (UAC_REC.g_au32UAC_RingBuff[UAC_REC.g_u16UAC_Buff_ReadIndex] >>  0) & 0xff;		
            pu8buf[i*4+1] = (UAC_REC.g_au32UAC_RingBuff[UAC_REC.g_u16UAC_Buff_ReadIndex] >>  8) & 0xff;		

            pu8buf[i*4+2] = (UAC_REC.g_au32UAC_RingBuff[UAC_REC.g_u16UAC_Buff_ReadIndex] >> 16) & 0xff;		
            pu8buf[i*4+3] = (UAC_REC.g_au32UAC_RingBuff[UAC_REC.g_u16UAC_Buff_ReadIndex] >> 24) & 0xff;		
            UAC_REC.g_u16UAC_Buff_ReadIndex++;	

            if ( UAC_REC.g_u16UAC_Buff_ReadIndex == AMIC_RING_BUFFER_LEN_34CH ) 
                UAC_REC.g_u16UAC_Buff_ReadIndex = 0;
            
        }	
          
        
		USBD_SET_PAYLOAD_LEN(EP5, u8EP5_Byte_Count);	
        
	}
	else
	{	
		USBD_SET_PAYLOAD_LEN(EP5, 0);
	}
    
    
}

void AUDIO_Init(void)
{
    int32_t i;
    uint8_t *pu8;
    uint8_t *pSerial = __TIME__;

    // Init setup packet buffer 
    // Buffer range for setup packet -> [0 ~ 0x7] 
    USBD->STBUFSEG = SETUP_BUF_BASE;

    //***************************************************
    // EP0 ==> control IN endpoint, address 0 
    USBD_CONFIG_EP(EP0, USBD_CFG_CSTALL | USBD_CFG_EPMODE_IN | 0);
    // Buffer range for EP0 
    USBD_SET_EP_BUF_ADDR(EP0, EP0_BUF_BASE);

    // EP1 ==> control OUT endpoint, address 0 
    USBD_CONFIG_EP(EP1, USBD_CFG_CSTALL | USBD_CFG_EPMODE_OUT | 0);
    // Buffer range for EP1 
    USBD_SET_EP_BUF_ADDR(EP1, EP1_BUF_BASE);

    //***************************************************
    // EP2 ==> Iso IN endpoint, address 2 
    USBD_CONFIG_EP(EP2, USBD_CFG_EPMODE_IN | USBD_CFG_TYPE_ISO | EP2);
    // Buffer range for EP2 
    USBD_SET_EP_BUF_ADDR(EP2, EP2_BUF_BASE);
    
    // EP3 ==> Iso IN endpoint, address 3 
    USBD_CONFIG_EP(EP3, USBD_CFG_EPMODE_IN | USBD_CFG_TYPE_ISO | EP3);
    // Buffer range for EP3 
    USBD_SET_EP_BUF_ADDR(EP3, EP3_BUF_BASE);
    
    // EP4 ==> Iso IN endpoint, address 4 
    USBD_CONFIG_EP(EP4, USBD_CFG_EPMODE_IN | USBD_CFG_TYPE_ISO | EP4);
    // Buffer range for EP4 
    USBD_SET_EP_BUF_ADDR(EP4, EP4_BUF_BASE);
    
    // EP5 ==> Iso IN endpoint, address 5 
    USBD_CONFIG_EP(EP5, USBD_CFG_EPMODE_IN | USBD_CFG_TYPE_ISO | EP5);
    // Buffer range for EP5
    USBD_SET_EP_BUF_ADDR(EP5, EP5_BUF_BASE);
    
    


    /*
       Generate Mass-Storage Device serial number
       To compliant USB-IF MSC test, we must enable serial string descriptor.
       However, window may fail to recognize the devices if PID/VID and serial number are all the same
       when plug them to Windows at the sample time.
       Therefore, we must generate different serial number for each device to avoid conflict
       when plug more then 2 MassStorage devices to Windows at the same time.

       NOTE: We use compiler predefine macro "__TIME__" to generate different number for serial
       at each build but each device here for a demo.
       User must change it to make sure all serial number is different between each device.
     */
        pu8 = (uint8_t *)gsInfo.gu8StringDesc[3];
		
        for(i = 0; i < 8; i++)
        pu8[pu8[0] - 16 + i * 2] = pSerial[i];

}

void AUDIO_ClassRequest(void)
{
    uint8_t buf[8];

    USBD_GetSetupPacket(buf);
    
    if((buf[0] & 0x7f) == 0x22) 	// bmRequestType= class, endpoint
	{ 
		if(buf[0] & 0x80)		    // Device to host
		{
			switch(buf[1])
			{
                case UAC_GET_CUR:
                {
                    if ( buf[3]==SAMPLING_FREQ_CONTROL && (buf[4]==EP3))
					{
                      //CX  g_u8RecEn = 0;
                        M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 0) = (uint8_t)UAC_REC.g_usbd_PlaySampleRate;
                        M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = (uint8_t)(UAC_REC.g_usbd_PlaySampleRate >> 8);
                        M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 2) = (uint8_t)(UAC_REC.g_usbd_PlaySampleRate >>16);
                        
                        // Data stage 
                        USBD_SET_DATA1(EP0);
                        USBD_SET_PAYLOAD_LEN(EP0, 3);
                        printf("Get_CUR: %08X\n", UAC_REC.g_usbd_PlaySampleRate); 

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
                         // error endpoint or un identified Control
                         // Setup error, stall the device 
                         USBD_SetStall(0);
						 printf("Get_CUR USBD_SetStall(0): %08X\n", UAC_REC.g_usbd_PlaySampleRate); 
                    }
                   
					// Trigger next Control Out DATA1 Transaction.
					// Status stage 
					USBD_PrepareCtrlOut(0, 0);
					break;
                }
								 
                default:
                {
                    // Setup error, stall the device 
                    //unidentify CONTROL
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
                        USBD_PrepareCtrlOut((uint8_t *)&UAC_REC.g_usbd_PlaySampleRate, buf[6]);
												                            
                        // Status stage 
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
                        // Setup error, stall the device 
                        // Unidentify CONTROL
                        // STALL control pipe 
                        USBD_SetStall(0);
                    }

                    break;
                }
                default:
                {
                    //unimplement CONTROL or wrong endpoint number
                    // Setup error, stall the device 
                    USBD_SetStall(0);
                }    
             }
        }
        
    }
    else		//Feature unit control
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
//                            #if (UAC_MIC_DEBUG)
//                            printf("MUTE_CONTROL");
//                            #endif
                            
                            if(REC_FEATURE_UNITID_1CH == buf[5])
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecMute_1CH;
                            else if(REC_FEATURE_UNITID_2CH == buf[5])
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecMute_2CH;
                            else if(REC_FEATURE_UNITID_3CH == buf[5])
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecMute_3CH;
                            else if(REC_FEATURE_UNITID_4CH == buf[5])
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecMute_4CH;
                            else if(PLAY_FEATURE_UNITID == buf[5])
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_PlayMute;

                            // Data stage 
                            USBD_SET_DATA1(EP0);
                            USBD_SET_PAYLOAD_LEN(EP0, 1);
                            break;
                        }
                        case VOLUME_CONTROL:
                        {
//                            #if (UAC_MIC_DEBUG)
//                            printf("VOLUME_CONTROL");
//                            #endif 
                            
                            if(REC_FEATURE_UNITID_1CH == buf[5])
                            {
                                // Mono channel 
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecVolume_1CH;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecVolume_1CH >> 8;
                            }
                            else if(REC_FEATURE_UNITID_2CH == buf[5])
                            {
                                // Left or right channel 
                                if(buf[2] == 1)
                                {
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecVolume_2CH[0];
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecVolume_2CH[0] >> 8;
                                }
                                else
                                {
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecVolume_2CH[1];
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecVolume_2CH[1] >> 8;
                                }

                            }
                            else if(REC_FEATURE_UNITID_3CH == buf[5])
                            {
                                // Left or right channel 
                                if(buf[2] == 1)
                                {
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecVolume_3CH[0];
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecVolume_3CH[0] >> 8;
                                }
                                else if(buf[2] == 2)
                                {
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecVolume_3CH[1];
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecVolume_3CH[1] >> 8;
                                }
                                else if(buf[2] == 3)
                                {
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecVolume_3CH[2];
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecVolume_3CH[2] >> 8;
                                }

                            }
                            else if(REC_FEATURE_UNITID_4CH == buf[5])
                            {
                                // Left or right channel 
                                if(buf[2] == 1)
                                {
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecVolume_4CH[0];
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecVolume_4CH[0] >> 8;
                                }
                                else if(buf[2] == 2)
                                {
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecVolume_4CH[1];
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecVolume_4CH[1] >> 8;
                                }
                                else if(buf[2] == 3)
                                {
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecVolume_4CH[2];
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecVolume_4CH[2] >> 8;
                                }
                                else if(buf[2] == 4)
                                {
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecVolume_4CH[3];
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecVolume_4CH[3] >> 8;
                                }
                                

                            }
                            else if(PLAY_FEATURE_UNITID == buf[5])
                            {
                                // Left or right channel 
                                if(buf[2] == 1)
                                {
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_PlayVolumeL;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_PlayVolumeL >> 8;
                                }
                                else
                                {
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_PlayVolumeR;
                                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_PlayVolumeR >> 8;
                                }
                            }

                            // Data stage 
                            USBD_SET_DATA1(EP0);
                            USBD_SET_PAYLOAD_LEN(EP0, 2);
                            break;
                        }
                        default:
                        {
                            // Setup error, stall the device 
                            USBD_SetStall(0);
                        }
                    }

                    // Trigger next Control Out DATA1 Transaction.
                    // Status stage 
                    USBD_PrepareCtrlOut(0, 0);
                    break;
                }

                case UAC_GET_MIN:
                {
                    switch(buf[3])
                    {
                        case VOLUME_CONTROL:
                        {
                            if(REC_FEATURE_UNITID_1CH == buf[5])
                            {
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecMinVolume_1CH;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecMinVolume_1CH >> 8;
                            }
                            else if(REC_FEATURE_UNITID_2CH == buf[5])
                            {
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecMinVolume_2CH;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecMinVolume_2CH >> 8;
                            }
                            else if(REC_FEATURE_UNITID_3CH == buf[5])
                            {
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecMinVolume_3CH;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecMinVolume_3CH >> 8;
                            }
                            else if(REC_FEATURE_UNITID_4CH == buf[5])
                            {
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecMinVolume_4CH;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecMinVolume_4CH >> 8;
                            }
                            else if(PLAY_FEATURE_UNITID == buf[5])
                            {
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_PlayMinVolume;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_PlayMinVolume >> 8;
                            }
                            // Data stage 
                            USBD_SET_DATA1(EP0);
                            USBD_SET_PAYLOAD_LEN(EP0, 2);
                            break;
                        }
                        default:
                            // STALL control pipe 
                            USBD_SetStall(0);
                    }
                    // Trigger next Control Out DATA1 Transaction.
                    // Status stage 
                    USBD_PrepareCtrlOut(0, 0);
                    break;
                }

                case UAC_GET_MAX:
                {
                    switch(buf[3])
                    {
                        case VOLUME_CONTROL:
                        {
                            if(REC_FEATURE_UNITID_1CH == buf[5])
                            {
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecMaxVolume_1CH;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecMaxVolume_1CH >> 8;
                            }
                            else if(REC_FEATURE_UNITID_2CH == buf[5])
                            {
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecMaxVolume_2CH;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecMaxVolume_2CH >> 8;
                            }
                            else if(REC_FEATURE_UNITID_3CH == buf[5])
                            {
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecMaxVolume_3CH;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecMaxVolume_3CH >> 8;
                            }
                            else if(REC_FEATURE_UNITID_4CH == buf[5])
                            {
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecMaxVolume_4CH;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecMaxVolume_4CH >> 8;
                            }
                            else if(PLAY_FEATURE_UNITID == buf[5])
                            {
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_PlayMaxVolume;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_PlayMaxVolume >> 8;
                            }
                            // Data stage 
                            USBD_SET_DATA1(EP0);
                            USBD_SET_PAYLOAD_LEN(EP0, 2);
                            break;
                        }
                        default:
                            // STALL control pipe 
                            USBD_SetStall(0);
                    }
                                        
                    // Trigger next Control Out DATA1 Transaction.
                    // Status stage 
                    USBD_PrepareCtrlOut(0, 0);
                    break;
                }

                case UAC_GET_RES:
                {
                    switch(buf[3])
                    {
                        case VOLUME_CONTROL:
                        {
                            if(REC_FEATURE_UNITID_1CH == buf[5])
                            {
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecResVolume_1CH;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecResVolume_1CH >> 8;
                            }
                            else if(REC_FEATURE_UNITID_2CH == buf[5])
                            {
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecResVolume_2CH;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecResVolume_2CH >> 8;
                            }
                            else if(REC_FEATURE_UNITID_3CH == buf[5])
                            {
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecResVolume_3CH;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecResVolume_3CH >> 8;
                            }
                            else if(REC_FEATURE_UNITID_4CH == buf[5])
                            {
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_RecResVolume_4CH;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_RecResVolume_4CH >> 8;
                            }
                            else if(PLAY_FEATURE_UNITID == buf[5])
                            {
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0)) = UAC_REC.g_usbd_PlayResVolume;
                                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP0) + 1) = UAC_REC.g_usbd_PlayResVolume >> 8;
                            }
                            // Data stage 
                            USBD_SET_DATA1(EP0);
                            USBD_SET_PAYLOAD_LEN(EP0, 2);
                            break;
                        }
                        default:
                            // STALL control pipe 
                            USBD_SetStall(0);
                    }
                    // Trigger next Control Out DATA1 Transaction.
                    // Status stage 
                    USBD_PrepareCtrlOut(0, 0);
                    break;
                }
                                

                default:
                {
                    // Setup error, stall the device 
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
                            if(REC_FEATURE_UNITID_1CH == buf[5])
                            {
                                #if (UAC_MIC_DEBUG)
                                printf("MUTE_CONTROL_1CH\n");
                                #endif
                                USBD_PrepareCtrlOut((uint8_t *)&UAC_REC.g_usbd_RecMute_1CH, buf[6]);
                            }
                            else if(REC_FEATURE_UNITID_2CH == buf[5])
                            {
                                #if (UAC_MIC_DEBUG)
                                printf("MUTE_CONTROL_2CH\n");
                                #endif
                                USBD_PrepareCtrlOut((uint8_t *)&UAC_REC.g_usbd_RecMute_2CH, buf[6]);
                            }
                            else if(REC_FEATURE_UNITID_3CH == buf[5])
                            {
                                #if (UAC_MIC_DEBUG)
                                printf("MUTE_CONTROL_3CH\n");
                                #endif
                                USBD_PrepareCtrlOut((uint8_t *)&UAC_REC.g_usbd_RecMute_3CH, buf[6]);
                            }
                            else if(REC_FEATURE_UNITID_4CH == buf[5])
                            {
                                #if (UAC_MIC_DEBUG)
                                printf("MUTE_CONTROL_4CH\n");
                                #endif
                                USBD_PrepareCtrlOut((uint8_t *)&UAC_REC.g_usbd_RecMute_4CH, buf[6]);
                            }
                            else if(PLAY_FEATURE_UNITID == buf[5])
                            {
                                USBD_PrepareCtrlOut((uint8_t *)&UAC_REC.g_usbd_PlayMute, buf[6]);
                            }
                            
                            // Status stage 
                            USBD_SET_DATA1(EP0);
                            USBD_SET_PAYLOAD_LEN(EP0, 0);
                            break;

                        case VOLUME_CONTROL:    // 0x02
                            if(REC_FEATURE_UNITID_1CH == buf[5])
                            {
                                #if (UAC_MIC_DEBUG)
                                printf("VOLUME_CONTROL_1CH\n");
                                #endif
                                
                                // Prepare the buffer for new record volume of right channel 
                                USBD_PrepareCtrlOut((uint8_t *)&UAC_REC.g_usbd_RecVolume_1CH, buf[6]);
                                //printf("RecVolumeR_1CH = %x\n", UAC_REC.g_usbd_RecVolumeR_1CH);
                                
                            }
                            else if(REC_FEATURE_UNITID_2CH == buf[5])
                            {
                                #if (UAC_MIC_DEBUG)
                                printf("VOLUME_CONTROL_2CH = %x\n", buf[2]);
                                #endif
                                
                                if(buf[2] == 1)
                                {
                                    // Prepare the buffer for new record volume of left channel 
                                    USBD_PrepareCtrlOut((uint8_t *)&UAC_REC.g_usbd_RecVolume_2CH[0], buf[6]);
                                }
                                else if(buf[2] == 2)
                                {
                                    // Prepare the buffer for new record volume of right channel 
                                    USBD_PrepareCtrlOut((uint8_t *)&UAC_REC.g_usbd_RecVolume_2CH[1], buf[6]);
                                }
                            }
                            else if(REC_FEATURE_UNITID_3CH == buf[5])
                            {
                                #if (UAC_MIC_DEBUG)
                                printf("VOLUME_CONTROL_3CH = %x\n", buf[2]);
                                #endif
                                
                                if(buf[2] == 1)
                                {
                                    // Prepare the buffer for new record volume of left channel 
                                    USBD_PrepareCtrlOut((uint8_t *)&UAC_REC.g_usbd_RecVolume_3CH[0], buf[6]);
                                }
                                else if(buf[2] == 2)
                                {
                                    // Prepare the buffer for new record volume of right channel 
                                    USBD_PrepareCtrlOut((uint8_t *)&UAC_REC.g_usbd_RecVolume_3CH[1], buf[6]);
                                }
                                else if(buf[2] == 3)
                                {
                                    // Prepare the buffer for new record volume of right channel 
                                    USBD_PrepareCtrlOut((uint8_t *)&UAC_REC.g_usbd_RecVolume_3CH[2], buf[6]);
                                    
                                }
                            }
                            else if(REC_FEATURE_UNITID_4CH == buf[5])
                            {
                                #if (UAC_MIC_DEBUG)
                                printf("VOLUME_CONTROL_4CH = %x\n", buf[2]);
                                #endif
                                
                                if(buf[2] == 1)
                                {
                                    // Prepare the buffer for new record volume of left channel 
                                    USBD_PrepareCtrlOut((uint8_t *)&UAC_REC.g_usbd_RecVolume_4CH[0], buf[6]);
                                }
                                else if(buf[2] == 2)
                                {
                                    // Prepare the buffer for new record volume of left channel 
                                    USBD_PrepareCtrlOut((uint8_t *)&UAC_REC.g_usbd_RecVolume_4CH[1], buf[6]);
                                }
                                else if(buf[2] == 3)
                                {
                                    // Prepare the buffer for new record volume of left channel 
                                    USBD_PrepareCtrlOut((uint8_t *)&UAC_REC.g_usbd_RecVolume_4CH[2], buf[6]);
                                }
                                else if(buf[2] == 4)
                                {
                                    // Prepare the buffer for new record volume of right channel 
                                    USBD_PrepareCtrlOut((uint8_t *)&UAC_REC.g_usbd_RecVolume_4CH[3], buf[6]);
                                }
                            }
                            else if(PLAY_FEATURE_UNITID == buf[5])
                            {

                                if(buf[2] == 1)
                                {
                                    // Prepare the buffer for new play volume of left channel 
                                    USBD_PrepareCtrlOut((uint8_t *)&UAC_REC.g_usbd_PlayVolumeL, buf[6]);
                                }
                                else
                                {
                                    // Prepare the buffer for new play volume of right channel 
                                    USBD_PrepareCtrlOut((uint8_t *)&UAC_REC.g_usbd_PlayVolumeR, buf[6]);
                                }
                            }

                            // Status stage 
                            USBD_SET_DATA1(EP0);
                            USBD_SET_PAYLOAD_LEN(EP0, 0);
                            break;

                        default:
                            // STALL control pipe 
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
                        // Request Type = Output 
                        USBD_SET_DATA1(EP1);
                        USBD_SET_PAYLOAD_LEN(EP1, buf[6]);

                        // Status stage 
                        USBD_PrepareCtrlIn(0, 0);
                    }
                    break;
                }
                case HID_SET_IDLE:
                {
                    // Status stage 
                    USBD_SET_DATA1(EP0);
                    USBD_SET_PAYLOAD_LEN(EP0, 0);
                    break;
                }
                case HID_SET_PROTOCOL:

                default:
                {
                    // Setup error, stall the device 
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

    USBD_GetSetupPacket(buf);

    u32AltInterface = buf[2];
    g_usbd_UsbInterface = buf[4];
	
    if ( g_usbd_UsbInterface == INTERFACE_NUMBER_1CH )		 
    {
        if (u32AltInterface == 1)
        {
            UAC_REC.g_u8Current_Mic_ChannNum = UAC_REC_CHANNEL_1CH;
            UAC_REC.g_usbd_UsbAudioState = UAC_START_AUDIO_RECORD;
                        
            USBD_SET_DATA1(EP2);
            USBD_SET_PAYLOAD_LEN(EP2, 0);	
                
            UAC_REC.g_u16UAC_Buff_WriteIndex = 0;
            UAC_REC.g_u16UAC_Buff_ReadIndex = 0;	
            
            PDMA_DISABLE_CHANNEL(PDMA_CH3_MASK);
            I2S_PDMA_Init(UAC_REC_CHANNEL_1CH);
            MIC_Start();
        }
        else
        {
            UAC_REC.g_usbd_UsbAudioState = UAC_STOP_AUDIO_RECORD;
            USBD_SET_DATA1(EP2);
            USBD_SET_PAYLOAD_LEN(EP2, 0);
            MIC_Stop();
        }
    }
    else if ( g_usbd_UsbInterface == INTERFACE_NUMBER_2CH )		 
    {
        if (u32AltInterface == 1)
        {
            UAC_REC.g_u8Current_Mic_ChannNum = UAC_REC_CHANNEL_2CH;
            UAC_REC.g_usbd_UsbAudioState = UAC_START_AUDIO_RECORD;
                        
            USBD_SET_DATA1(EP3);
            USBD_SET_PAYLOAD_LEN(EP3, 0);	
                
            UAC_REC.g_u16UAC_Buff_WriteIndex = 0;
            UAC_REC.g_u16UAC_Buff_ReadIndex = 0;
            PDMA_DISABLE_CHANNEL(PDMA_CH3_MASK);    
            I2S_PDMA_Init(UAC_REC_CHANNEL_2CH);
            MIC_Start();
        }
        else
        {
            UAC_REC.g_usbd_UsbAudioState = UAC_STOP_AUDIO_RECORD;
            USBD_SET_DATA1(EP3);
            USBD_SET_PAYLOAD_LEN(EP3, 0);
            MIC_Stop();
        }
    }
    else if ( g_usbd_UsbInterface == INTERFACE_NUMBER_3CH )		 
    {
        if (u32AltInterface == 1)
        {
            UAC_REC.g_u8Current_Mic_ChannNum = UAC_REC_CHANNEL_3CH;
            UAC_REC.g_usbd_UsbAudioState = UAC_START_AUDIO_RECORD;
                        
            USBD_SET_DATA1(EP4);
            USBD_SET_PAYLOAD_LEN(EP4, 0);	
                
            UAC_REC.g_u16UAC_Buff_WriteIndex = 0;
            UAC_REC.g_u16UAC_Buff_ReadIndex = 0;	
            PDMA_DISABLE_CHANNEL(PDMA_CH3_MASK);
            I2S_PDMA_Init(UAC_REC_CHANNEL_3CH);
            MIC_Start();
        }
        else
        {
            UAC_REC.g_usbd_UsbAudioState = UAC_STOP_AUDIO_RECORD;
            USBD_SET_DATA1(EP4);
            USBD_SET_PAYLOAD_LEN(EP4, 0);
            MIC_Stop();
        }
    }
    else if ( g_usbd_UsbInterface == INTERFACE_NUMBER_4CH )		 
    {
        if (u32AltInterface == 1)
        {
            UAC_REC.g_u8Current_Mic_ChannNum = UAC_REC_CHANNEL_4CH;
            UAC_REC.g_usbd_UsbAudioState = UAC_START_AUDIO_RECORD;
                        
            USBD_SET_DATA1(EP5);
            USBD_SET_PAYLOAD_LEN(EP5, 0);	
                
            UAC_REC.g_u16UAC_Buff_WriteIndex = 0;
            UAC_REC.g_u16UAC_Buff_ReadIndex = 0;	
            PDMA_DISABLE_CHANNEL(PDMA_CH3_MASK);
            I2S_PDMA_Init(UAC_REC_CHANNEL_4CH);
            MIC_Start();
        }
        else
        {
            UAC_REC.g_usbd_UsbAudioState = UAC_STOP_AUDIO_RECORD;
            USBD_SET_DATA1(EP5);
            USBD_SET_PAYLOAD_LEN(EP5, 0);
            MIC_Stop();
        }
    }        
        
}
