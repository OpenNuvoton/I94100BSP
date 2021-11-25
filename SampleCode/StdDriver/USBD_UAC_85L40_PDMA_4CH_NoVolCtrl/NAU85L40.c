/******************************************************************************
 * @file     NAU85L40.c
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
#include "BUFCTRL.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Define Constant                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
#define MIC_PDMA_16BIT

////////////////////////////////////////////////////////////////////////////////////
#define MIC_VOLUME_CTRL_OFFSET  (0x57FF)    
#define MIC_VOLUME_CTRL_RANGE   (0x7FFF-MIC_VOLUME_CTRL_OFFSET)     
#define NAU85L40_DGAIN_TOP      (20<<3)             
#define NAU85L40_DGAIN_RES      (512)             
#define MIC_VOLUME_CTRL_RATIO   (MIC_VOLUME_CTRL_RANGE/NAU85L40_DGAIN_RES)      
#define NAU85L40_DGAIN_STEPS    (NAU85L40_DGAIN_RES-NAU85L40_DGAIN_TOP)             
#define NAU85L40_DGAIN_OFFSET   (0x400-NAU85L40_DGAIN_STEPS)   

// I2C ============================================================================================
#define MIC_I2C1_PIN_MASK  (SYS_GPD_MFPH_PD14MFP_Msk|SYS_GPD_MFPH_PD15MFP_Msk)
#define MIC_I2C1_PIN       (SYS_GPD_MFPH_PD14MFP_I2C1_SCL|SYS_GPD_MFPH_PD15MFP_I2C1_SDA)
#define MIC_I2C1_BUS_FREQ  (100000)   
#define MIC_I2C1_DEV_ADDR  (0x1C)

#define MIC_PDMA_CH        (3)


// I2S ============================================================================================
#define MIC_I2S0_MCLK_FREQ (256*REC_RATE)    // 256*Fs
#define MIC_I2S0_PIN_MASK  (SYS_GPD_MFPL_PD2MFP_Msk|SYS_GPD_MFPL_PD3MFP_Msk|SYS_GPD_MFPL_PD4MFP_Msk|SYS_GPD_MFPL_PD5MFP_Msk|SYS_GPD_MFPL_PD6MFP_Msk)
#define MIC_I2S0_PIN       (SYS_GPD_MFPL_PD2MFP_I2S0_MCLK|SYS_GPD_MFPL_PD3MFP_I2S0_LRCK|SYS_GPD_MFPL_PD4MFP_I2S0_DI|SYS_GPD_MFPL_PD5MFP_I2S0_DO|SYS_GPD_MFPL_PD6MFP_I2S0_BCLK)

/*---------------------------------------------------------------------------------------------------------*/
/* Define Variables                                                                                        */
/*---------------------------------------------------------------------------------------------------------*/
typedef struct{
	uint8_t  u8DeviceAddr;
	uint16_t u16Counter;
	uint16_t u16MaxCount;
	uint8_t* pau8Cmd;
} S_MIC_I2CCTRL;

typedef struct{
	uint8_t  u8Reg[2];
	uint8_t  u8Value[2];
} S_MIC_I2CCMD;

DSCT_T sPDMA_MIC[2];
volatile S_MIC_I2CCTRL s_MIC_I2CCtrl;      	// Provide microphone send command to 85L40S_BUFCTRL* psSPK_BufCtrl = NULL;

// Microphone(85L40) Command =======================================================================
// Command for 85L40(transfer via I2C1)
S_MIC_I2CCMD const asMIC_Cmd_85L40[] = {
//-------
{	0x00	,	0x00	,	0x00	,	0x01	}	,
//-------
{	0x00	,	0x03	,	0x00	,	0x40	}	,
{	0x00	,	0x04	,	0x00	,	0x01	}	,
{	0x00	,	0x05	,	0x31	,	0x26	}	,
{	0x00	,	0x06	,	0x00	,	0x08	}	,
{	0x00	,	0x07	,	0x00	,	0x10	}	,
{	0x00	,	0x08	,	0xC0	,	0x00	}	,
{	0x00	,	0x09	,	0xE0	,	0x00	}	,
{	0x00	,	0x0A	,	0xF1	,	0x3C	}	,
#if (AMIC_BIT_RES == 32) 
{	0x00	,	0x10	,	0x00	,	0x4F	}	,		//PCMB, 32Bit
#endif 
#if (AMIC_BIT_RES == 16)
{	0x00	,	0x10	,	0x00	,	0x43	}	,		//PCMB, 16Bit
#endif 
{	0x00	,	0x11	,	0x00	,	0x00	}	,
{	0x00	,	0x12	,	0x00	,	0x00	}	,
{	0x00	,	0x13	,	0x00	,	0x00	}	,
{	0x00	,	0x14	,	0xC0	,	0x0F	}	,		// Enable 4 channels 	
{	0x00	,	0x20	,	0x00	,	0x00	}	,
{	0x00	,	0x21	,	0x70	,	0x0B	}	,
{	0x00	,	0x22	,	0x00	,	0x22	}	,
{	0x00	,	0x23	,	0x10	,	0x10	}	,
{	0x00	,	0x24	,	0x10	,	0x10	}	,
{	0x00	,	0x2D	,	0x10	,	0x10	}	,
{	0x00	,	0x2E	,	0x10	,	0x10	}	,
{	0x00	,	0x2F	,	0x00	,	0x00	}	,
{	0x00	,	0x30	,	0x00	,	0x00	}	,
{	0x00	,	0x31	,	0x00	,	0x00	}	,
{	0x00	,	0x32	,	0x00	,	0x00	}	,
{	0x00	,	0x33	,	0x00	,	0x00	}	,
{	0x00	,	0x34	,	0x00	,	0x00	}	,
{	0x00	,	0x35	,	0x00	,	0x00	}	,
{	0x00	,	0x36	,	0x00	,	0x00	}	,
{	0x00	,	0x37	,	0x00	,	0x00	}	,
{	0x00	,	0x38	,	0x00	,	0x00	}	,
{	0x00	,	0x39	,	0x00	,	0x00	}	,
{	0x00	,	0x3A	,	0x40	,	0x62	}	,		//16K SR
{	0x00	,	0x40	,	0x04	,	0x08	}	,  	//DGAIN = 0dB
{	0x00	,	0x41	,	0x04	,	0x08	}	,  	//DGAIN = 0dB
{	0x00	,	0x42	,	0x04	,	0x08	}	,  	//DGAIN = 0dB
{	0x00	,	0x43	,	0x04	,	0x08	}	,  	//DGAIN = 0dB
{	0x00	,	0x44	,	0x00	,	0xE4	}	,
{	0x00	,	0x48	,	0x00	,	0x00	}	,
{	0x00	,	0x49	,	0x00	,	0x00	}	,
{	0x00	,	0x4A	,	0x00	,	0x00	}	,
{	0x00	,	0x4B	,	0x00	,	0x00	}	,
{	0x00	,	0x4C	,	0x00	,	0x00	}	,
{	0x00	,	0x4D	,	0x00	,	0x00	}	,
{	0x00	,	0x4E	,	0x00	,	0x00	}	,
{	0x00	,	0x4F	,	0x00	,	0x00	}	,
{	0x00	,	0x50	,	0x00	,	0x00	}	,
{	0x00	,	0x51	,	0x00	,	0x00	}	,
{	0x00	,	0x52	,	0xEF	,	0xFF	}	,
{	0x00	,	0x57	,	0x00	,	0x00	}	,
{	0x00	,	0x58	,	0x1C	,	0xF0	}	,
{	0x00	,	0x59	,	0x00	,	0x08	}	,
{	0x00	,	0x60	,	0x00	,	0x60	}	,		// VMID_CTRL 
{	0x00	,	0x61	,	0x00	,	0x00	}	,
{	0x00	,	0x62	,	0x00	,	0x00	}	,
{	0x00	,	0x63	,	0x00	,	0x00	}	,
{	0x00	,	0x64	,	0x00	,	0x11	}	,
{	0x00	,	0x65	,	0x02	,	0x20	}	,
{	0x00	,	0x66	,	0x00	,	0x0F	}	,
{	0x00	,	0x67	,	0x0D	,	0x04	}	,
{	0x00	,	0x68	,	0x70	,	0x00	}	,
{	0x00	,	0x69	,	0x00	,	0x00	}	,
{	0x00	,	0x6A	,	0x00	,	0x00	}	,
//{	0x00	,	0x6B	,	0x1B	,	0x1B	}	,  //AGAIN = 26dB
//{	0x00	,	0x6C	,	0x1B	,	0x1B	}	,  //AGAIN = 26dB
{	0x00	,	0x6B	,	0x18	,	0x18	}	,  //AGAIN = 32dB
{	0x00	,	0x6C	,	0x18	,	0x18	}	,  //AGAIN = 32dB
{	0x00	,	0x6D	,	0xF0	,	0x00	}	,
{	0x00	,	0x01	,	0x00	,	0x0F	}	,
{	0x00	,	0x02	,	0x80	,	0x03	}	
};

/*---------------------------------------------------------------------------------------------------------*/
/* Functions Definition                                                                                    */
/*---------------------------------------------------------------------------------------------------------*/
void  NAU85L40_Vol_Ctrl_1CH(void)
{
	S_MIC_I2CCMD  mute, gain;
	int16_t i16temp=0;
	uint16_t u16temp=0;

	if (UAC_REC.g_usbd_RecMute_Cur_1CH != UAC_REC.g_usbd_RecMute_1CH)
	{
		UAC_REC.g_usbd_RecMute_Cur_1CH = UAC_REC.g_usbd_RecMute_1CH;
		
		mute.u8Reg[0] = 0x00;   
		mute.u8Reg[1] = 0x61; 
		if (UAC_REC.g_usbd_RecMute_Cur_1CH == 0x01)
		{
			mute.u8Value[0] = 0x00; 
			mute.u8Value[1] = 0x0f;  // mute enable 
		}
		else
		{
			mute.u8Value[0] = 0x00; 
			mute.u8Value[1] = 0x00;  
		}
		
		s_MIC_I2CCtrl.pau8Cmd = (uint8_t*)&mute.u8Reg[0];
		s_MIC_I2CCtrl.u16Counter = 0;
		s_MIC_I2CCtrl.u16MaxCount = sizeof(S_MIC_I2CCMD);
		I2C_START(I2C1);
		/* Wait for I2C transmit completed. */
		while(s_MIC_I2CCtrl.u16MaxCount>0);
	}            
	else if ( UAC_REC.g_usbd_RecVolume_Cur_1CH != UAC_REC.g_usbd_RecVolume_1CH)
	{
		UAC_REC.g_usbd_RecVolume_Cur_1CH = UAC_REC.g_usbd_RecVolume_1CH;
		
		// step1. Obtain gain setting of NAU85L40
		i16temp = UAC_REC.g_usbd_RecVolume_Cur_1CH;
		printf("volume value = 0x%x\n", i16temp);
		
		if ( i16temp > MIC_VOLUME_CTRL_OFFSET ) 
		{
			i16temp = i16temp - MIC_VOLUME_CTRL_OFFSET;  
			i16temp = i16temp / MIC_VOLUME_CTRL_RATIO;                                
		}
		else 
		{
			i16temp = 0;
		}                  
		u16temp = (uint16_t)(i16temp) + NAU85L40_DGAIN_OFFSET;    
			 
		// step2. Re-configure NAU85L40
		gain.u8Reg[0] = 0x00;
		gain.u8Reg[1] = 0x40; 
		gain.u8Value[0] = (u16temp >> 8 ) & 0xff;    
		gain.u8Value[1] = u16temp & 0xff;
		printf("gain.u8Value[0]=%x ;gain.u8Value[1]=%x\n", gain.u8Value[0], gain.u8Value[1]);

		s_MIC_I2CCtrl.pau8Cmd = (uint8_t*)&gain;
		s_MIC_I2CCtrl.u16Counter = 0;
		s_MIC_I2CCtrl.u16MaxCount = sizeof(S_MIC_I2CCMD);
		I2C_START(I2C1);
		/* Wait for I2C transmit completed. */
		while(s_MIC_I2CCtrl.u16MaxCount>0);
	}
}

void  NAU85L40_Vol_Ctrl_2CH(void)
{
	S_MIC_I2CCMD  mute, gain;  
	int16_t i16temp=0;
	uint16_t u16temp=0; 
	
	if (UAC_REC.g_usbd_RecMute_Cur_2CH != UAC_REC.g_usbd_RecMute_2CH)
	{
		UAC_REC.g_usbd_RecMute_Cur_2CH = UAC_REC.g_usbd_RecMute_2CH;
		
		mute.u8Reg[0] = 0x00;   
		mute.u8Reg[1] = 0x61; 
		if (UAC_REC.g_usbd_RecMute_Cur_2CH == 0x01)
		{
			mute.u8Value[0] = 0x00; 
			mute.u8Value[1] = 0x0f;  // mute enable 
		}
		else
		{
			mute.u8Value[0] = 0x00; 
			mute.u8Value[1] = 0x00;  
		}
		
		s_MIC_I2CCtrl.pau8Cmd = (uint8_t*)&mute.u8Reg[0];
		s_MIC_I2CCtrl.u16Counter = 0;
		s_MIC_I2CCtrl.u16MaxCount = sizeof(S_MIC_I2CCMD);
		I2C_START(I2C1);
		/* Wait for I2C transmit completed. */
		while(s_MIC_I2CCtrl.u16MaxCount>0);
	}                                 
	else if ( UAC_REC.g_usbd_RecVolume_Cur_2CH[0] != UAC_REC.g_usbd_RecVolume_2CH[0] )  
	{
		UAC_REC.g_usbd_RecVolume_Cur_2CH[0] = UAC_REC.g_usbd_RecVolume_2CH[0];
									
		// step1. Obtain gain setting of NAU85L40
		i16temp = UAC_REC.g_usbd_RecVolume_Cur_2CH[0];
		
		if ( i16temp > MIC_VOLUME_CTRL_OFFSET ) 
		{
			i16temp = i16temp - MIC_VOLUME_CTRL_OFFSET;  
			i16temp = i16temp / MIC_VOLUME_CTRL_RATIO;    
		}
		else 
		{
			i16temp = 0;
		}
		
		u16temp = (uint16_t)(i16temp) + NAU85L40_DGAIN_OFFSET;    

		// step2. Re-configure NAU85L40 here
		gain.u8Reg[0] = 0x00;
		gain.u8Reg[1] = 0x40;
		gain.u8Value[0] = (u16temp >> 8 ) & 0xff;   
		gain.u8Value[1] = u16temp & 0xff;
		
		s_MIC_I2CCtrl.pau8Cmd = (uint8_t*)&gain.u8Reg[0];
		s_MIC_I2CCtrl.u16Counter = 0;
		s_MIC_I2CCtrl.u16MaxCount = sizeof(S_MIC_I2CCMD);
		I2C_START(I2C1);
		/* Wait for I2C transmit completed. */
		while(s_MIC_I2CCtrl.u16MaxCount>0);	 
	}
	else if ( UAC_REC.g_usbd_RecVolume_Cur_2CH[1] != UAC_REC.g_usbd_RecVolume_2CH[1] )
	{
		UAC_REC.g_usbd_RecVolume_Cur_2CH[1] = UAC_REC.g_usbd_RecVolume_2CH[1];
										 
		// step1. Obtain gain setting of NAU85L40
		i16temp = UAC_REC.g_usbd_RecVolume_Cur_2CH[1];
		
		if ( i16temp > MIC_VOLUME_CTRL_OFFSET ) 
		{
			i16temp = i16temp - MIC_VOLUME_CTRL_OFFSET;  
			i16temp = i16temp / MIC_VOLUME_CTRL_RATIO;    
		}
		else 
		{
			i16temp = 0;
		}
		
		u16temp = (uint16_t)(i16temp) + NAU85L40_DGAIN_OFFSET;    

		// step2. Re-configure NAU85L40 
		gain.u8Reg[0] = 0x00;
		gain.u8Reg[1] = 0x41;
		gain.u8Value[0] = (u16temp >> 8 ) & 0xff;   
		gain.u8Value[1] = u16temp & 0xff;
		
		s_MIC_I2CCtrl.pau8Cmd = (uint8_t*)&gain.u8Reg[0];
		s_MIC_I2CCtrl.u16Counter = 0;
		s_MIC_I2CCtrl.u16MaxCount = sizeof(S_MIC_I2CCMD);
		I2C_START(I2C1);
		/* Wait for I2C transmit completed. */
		while(s_MIC_I2CCtrl.u16MaxCount>0);
	}
}

void  NAU85L40_Vol_Ctrl_3CH(void)
{
	S_MIC_I2CCMD  mute, gain;
	int16_t i16temp=0;
	uint16_t u16temp=0;
													
	if (UAC_REC.g_usbd_RecMute_Cur_3CH != UAC_REC.g_usbd_RecMute_3CH)
	{
		UAC_REC.g_usbd_RecMute_Cur_3CH = UAC_REC.g_usbd_RecMute_3CH;
		
		mute.u8Reg[0] = 0x00;   
		mute.u8Reg[1] = 0x61; 
		if (UAC_REC.g_usbd_RecMute_Cur_3CH == 0x01)
		{
			mute.u8Value[0] = 0x00; 
			mute.u8Value[1] = 0x0f;  // mute enable 
		}
		else
		{
			mute.u8Value[0] = 0x00; 
			mute.u8Value[1] = 0x00;  
		}
		
		s_MIC_I2CCtrl.pau8Cmd = (uint8_t*)&mute.u8Reg[0];
		s_MIC_I2CCtrl.u16Counter = 0;
		s_MIC_I2CCtrl.u16MaxCount = sizeof(S_MIC_I2CCMD);
		I2C_START(I2C1);
		/* Wait for I2C transmit completed. */
		while(s_MIC_I2CCtrl.u16MaxCount>0);
	}    
	else if ( UAC_REC.g_usbd_RecVolume_Cur_3CH[0] != UAC_REC.g_usbd_RecVolume_3CH[0] ) 
	{
		UAC_REC.g_usbd_RecVolume_Cur_3CH[0] = UAC_REC.g_usbd_RecVolume_3CH[0];
		
		// step1. Obtain gain setting of NAU85L40
		i16temp = UAC_REC.g_usbd_RecVolume_Cur_3CH[0];
		
		if ( i16temp > MIC_VOLUME_CTRL_OFFSET ) 
		{
			i16temp = i16temp - MIC_VOLUME_CTRL_OFFSET;  
			i16temp = i16temp / MIC_VOLUME_CTRL_RATIO;    
		}
		else 
		{
			i16temp = 0;
		}
		
		u16temp = (uint16_t)(i16temp) + NAU85L40_DGAIN_OFFSET;    

		// step2. Re-configure NAU85L40 
		gain.u8Reg[0] = 0x00;
		gain.u8Reg[1] = 0x40;
		gain.u8Value[0] = (u16temp >> 8 ) & 0xff;   
		gain.u8Value[1] = u16temp & 0xff;
								
		s_MIC_I2CCtrl.pau8Cmd = (uint8_t*)&gain.u8Reg[0];
		s_MIC_I2CCtrl.u16Counter = 0;
		s_MIC_I2CCtrl.u16MaxCount = sizeof(S_MIC_I2CCMD);
		I2C_START(I2C1);
		/* Wait for I2C transmit completed. */
		while(s_MIC_I2CCtrl.u16MaxCount>0);
	}
	else if ( UAC_REC.g_usbd_RecVolume_Cur_3CH[1] != UAC_REC.g_usbd_RecVolume_3CH[1] )    
	{
		UAC_REC.g_usbd_RecVolume_Cur_3CH[1] = UAC_REC.g_usbd_RecVolume_3CH[1];
		
		// step1. Obtain gain setting of NAU85L40
		i16temp = UAC_REC.g_usbd_RecVolume_Cur_3CH[1];
		
		if ( i16temp > MIC_VOLUME_CTRL_OFFSET ) 
		{
			i16temp = i16temp - MIC_VOLUME_CTRL_OFFSET;  
			i16temp = i16temp / MIC_VOLUME_CTRL_RATIO;    
		}
		else 
		{
			i16temp = 0;
		}
		
		u16temp = (uint16_t)(i16temp) + NAU85L40_DGAIN_OFFSET;    

		// step2. Re-configure NAU85L40 here
		gain.u8Reg[0] = 0x00;
		gain.u8Reg[1] = 0x41;
		gain.u8Value[0] = (u16temp >> 8 ) & 0xff;   
		gain.u8Value[1] = u16temp & 0xff;
	 
		s_MIC_I2CCtrl.pau8Cmd = (uint8_t*)&gain.u8Reg[0];
		s_MIC_I2CCtrl.u16Counter = 0;
		s_MIC_I2CCtrl.u16MaxCount = sizeof(S_MIC_I2CCMD);
		I2C_START(I2C1);
		/* Wait for I2C transmit completed. */
		while(s_MIC_I2CCtrl.u16MaxCount>0);
	}
	else if ( UAC_REC.g_usbd_RecVolume_Cur_3CH[2] != UAC_REC.g_usbd_RecVolume_3CH[2] ) 
	{
		UAC_REC.g_usbd_RecVolume_Cur_3CH[2] = UAC_REC.g_usbd_RecVolume_3CH[2];
		
		// step1. Obtain gain setting of NAU85L40
		i16temp = UAC_REC.g_usbd_RecVolume_Cur_3CH[2];
		
		if ( i16temp > MIC_VOLUME_CTRL_OFFSET ) 
		{
			i16temp = i16temp - MIC_VOLUME_CTRL_OFFSET;  
			i16temp = i16temp / MIC_VOLUME_CTRL_RATIO;    
		}
		else 
		{
			i16temp = 0;
		}
		
		u16temp = (uint16_t)(i16temp) + NAU85L40_DGAIN_OFFSET;    

		// step2. Re-configure NAU85L40 
		gain.u8Reg[0] = 0x00;
		gain.u8Reg[1] = 0x42;
		gain.u8Value[0] = (u16temp >> 8 ) & 0xff;   
		gain.u8Value[1] = u16temp & 0xff;
		
		s_MIC_I2CCtrl.pau8Cmd = (uint8_t*)&gain.u8Reg[0];
		s_MIC_I2CCtrl.u16Counter = 0;
		s_MIC_I2CCtrl.u16MaxCount = sizeof(S_MIC_I2CCMD);
		I2C_START(I2C1);
		/* Wait for I2C transmit completed. */
		while(s_MIC_I2CCtrl.u16MaxCount>0);        
	}
}

void  NAU85L40_Vol_Ctrl_4CH(void)
{
	S_MIC_I2CCMD  mute, gain;  // gain[4];
	int16_t i16temp=0;
	uint16_t u16temp=0;    
	
	if (UAC_REC.g_usbd_RecMute_Cur_4CH != UAC_REC.g_usbd_RecMute_4CH)
	{
		UAC_REC.g_usbd_RecMute_Cur_4CH = UAC_REC.g_usbd_RecMute_4CH;
		
		mute.u8Reg[0] = 0x00;   
		mute.u8Reg[1] = 0x61; 
		if (UAC_REC.g_usbd_RecMute_Cur_4CH == 0x01)
		{
			mute.u8Value[0] = 0x00; 
			mute.u8Value[1] = 0x0f;  // mute enable 
		}
		else
		{
			mute.u8Value[0] = 0x00; 
			mute.u8Value[1] = 0x00;  
		}
		
		s_MIC_I2CCtrl.pau8Cmd = (uint8_t*)&mute.u8Reg[0];
		s_MIC_I2CCtrl.u16Counter = 0;
		s_MIC_I2CCtrl.u16MaxCount = sizeof(S_MIC_I2CCMD);
		I2C_START(I2C1);
		/* Wait for I2C transmit completed. */
		while(s_MIC_I2CCtrl.u16MaxCount>0);
	}                            
	else if ( UAC_REC.g_usbd_RecVolume_Cur_4CH[0] != UAC_REC.g_usbd_RecVolume_4CH[0] ) 
	{
		UAC_REC.g_usbd_RecVolume_Cur_4CH[0] = UAC_REC.g_usbd_RecVolume_4CH[0];
		
		// step1. Obtain gain setting of NAU85L40
		i16temp = UAC_REC.g_usbd_RecVolume_Cur_4CH[0];
		
		if ( i16temp > MIC_VOLUME_CTRL_OFFSET ) 
		{
			i16temp = i16temp - MIC_VOLUME_CTRL_OFFSET;  
			i16temp = i16temp / MIC_VOLUME_CTRL_RATIO;    
		}
		else 
		{
			i16temp = 0;
		}
		
		u16temp = (uint16_t)(i16temp) + NAU85L40_DGAIN_OFFSET;    

		// step2. Re-configure NAU85L40 
		gain.u8Reg[0] = 0x00;
		gain.u8Reg[1] = 0x40;
		gain.u8Value[0] = (u16temp >> 8 ) & 0xff;   
		gain.u8Value[1] = u16temp & 0xff;
		
		s_MIC_I2CCtrl.pau8Cmd = (uint8_t*)&gain.u8Reg[0];
		s_MIC_I2CCtrl.u16Counter = 0;
		s_MIC_I2CCtrl.u16MaxCount = sizeof(S_MIC_I2CCMD);
		I2C_START(I2C1);
		/* Wait for I2C transmit completed. */
		while(s_MIC_I2CCtrl.u16MaxCount>0);
	}
	else if ( UAC_REC.g_usbd_RecVolume_Cur_4CH[1] != UAC_REC.g_usbd_RecVolume_4CH[1] )
	{
		UAC_REC.g_usbd_RecVolume_Cur_4CH[1] = UAC_REC.g_usbd_RecVolume_4CH[1];
 
		// step1. Obtain gain setting of NAU85L40
		i16temp = UAC_REC.g_usbd_RecVolume_Cur_4CH[1];
		
		if ( i16temp > MIC_VOLUME_CTRL_OFFSET )
		{
			i16temp = i16temp - MIC_VOLUME_CTRL_OFFSET;  
			i16temp = i16temp / MIC_VOLUME_CTRL_RATIO;    
		}
		else 
		{
			i16temp = 0;
		}
		
		u16temp = (uint16_t)(i16temp) + NAU85L40_DGAIN_OFFSET;    

		// step2. Re-configure NAU85L40 
		gain.u8Reg[0] = 0x00;
		gain.u8Reg[1] = 0x41;
		gain.u8Value[0] = (u16temp >> 8 ) & 0xff;   
		gain.u8Value[1] = u16temp & 0xff;
		
		s_MIC_I2CCtrl.pau8Cmd = (uint8_t*)&gain.u8Reg[0];
		s_MIC_I2CCtrl.u16Counter = 0;
		s_MIC_I2CCtrl.u16MaxCount = sizeof(S_MIC_I2CCMD);
		I2C_START(I2C1);
		/* Wait for I2C transmit completed. */
		while(s_MIC_I2CCtrl.u16MaxCount>0);    
	}
	else if ( UAC_REC.g_usbd_RecVolume_Cur_4CH[2] != UAC_REC.g_usbd_RecVolume_4CH[2] ) 
	{	
		UAC_REC.g_usbd_RecVolume_Cur_4CH[2] = UAC_REC.g_usbd_RecVolume_4CH[2];
		
		// step1. Obtain gain setting of NAU85L40
		i16temp = UAC_REC.g_usbd_RecVolume_Cur_4CH[2];
		
		if ( i16temp > MIC_VOLUME_CTRL_OFFSET )
		{
			i16temp = i16temp - MIC_VOLUME_CTRL_OFFSET;  
			i16temp = i16temp / MIC_VOLUME_CTRL_RATIO;    
		}
		else 
		{
			i16temp = 0;
		}
		
		u16temp = (uint16_t)(i16temp) + NAU85L40_DGAIN_OFFSET;    

		// step2. Re-configure NAU85L40 
		gain.u8Reg[0] = 0x00;
		gain.u8Reg[1] = 0x42;
		gain.u8Value[0] = (u16temp >> 8 ) & 0xff;   
		gain.u8Value[1] = u16temp & 0xff;
		
		s_MIC_I2CCtrl.pau8Cmd = (uint8_t*)&gain.u8Reg[0];
		s_MIC_I2CCtrl.u16Counter = 0;
		s_MIC_I2CCtrl.u16MaxCount = sizeof(S_MIC_I2CCMD);
		I2C_START(I2C1);
		/* Wait for I2C transmit completed. */
		while(s_MIC_I2CCtrl.u16MaxCount>0);
	}
	else if ( UAC_REC.g_usbd_RecVolume_Cur_4CH[3] != UAC_REC.g_usbd_RecVolume_4CH[3] )
	{
		UAC_REC.g_usbd_RecVolume_Cur_4CH[3] = UAC_REC.g_usbd_RecVolume_4CH[3]; 
		
		// step1. Obtain gain setting of NAU85L40
		i16temp = UAC_REC.g_usbd_RecVolume_Cur_4CH[3];
		
		if ( i16temp > MIC_VOLUME_CTRL_OFFSET )
		{
			i16temp = i16temp - MIC_VOLUME_CTRL_OFFSET;   
			i16temp = i16temp / MIC_VOLUME_CTRL_RATIO;    
		}
		else 
		{
			i16temp = 0;
		}
		
		u16temp = (uint16_t)(i16temp) + NAU85L40_DGAIN_OFFSET;    

		// step2. Re-configure NAU85L40 
		gain.u8Reg[0] = 0x00;
		gain.u8Reg[1] = 0x43;
		gain.u8Value[0] = (u16temp >> 8 ) & 0xff;   
		gain.u8Value[1] = u16temp & 0xff;
				
		s_MIC_I2CCtrl.pau8Cmd = (uint8_t*)&gain.u8Reg[0];
		s_MIC_I2CCtrl.u16Counter = 0;
		s_MIC_I2CCtrl.u16MaxCount = sizeof(S_MIC_I2CCMD);
		I2C_START(I2C1);
		/* Wait for I2C transmit completed. */
		while(s_MIC_I2CCtrl.u16MaxCount>0);
	}
}


void NAU85L40_MuteVol_Ctrl(uint8_t ChannelNum)
{ 
	switch (ChannelNum)
	{
		case UAC_REC_CHANNEL_1CH:     
			NAU85L40_Vol_Ctrl_1CH();
		break;
		case UAC_REC_CHANNEL_2CH:     
			NAU85L40_Vol_Ctrl_2CH();
		break;
		case UAC_REC_CHANNEL_3CH:    
			NAU85L40_Vol_Ctrl_3CH();
		break;
		case UAC_REC_CHANNEL_4CH:     
			NAU85L40_Vol_Ctrl_4CH();
		break;       
		default:
		break;      
	}    
}

// ==========================================================================================
void MIC_Start(void)
{
	// Open PDMA channel
	PDMA_Open((1<<MIC_PDMA_CH));
	// Set TransMode
	PDMA_SetTransferMode(MIC_PDMA_CH, PDMA_I2S0_RX, TRUE, (uint32_t)&sPDMA_MIC[0]);
	// Enable interrupt
	PDMA_EnableInt(MIC_PDMA_CH,PDMA_INT_TRANS_DONE);

	I2S_ENABLE(I2S0);
	I2S_ENABLE_RX(I2S0);	
}

void MIC_Stop(void)
{
	I2S_DISABLE_RX(I2S0);
	I2S_DISABLE(I2S0);
}

void NAU85L40_Init(void)
{
	// (1) Configure I2C1 for sending command to 85L40
	// (1-1) Initiate I2C1
	{
		// Reset module. 
		SYS_ResetModule(I2C1_RST);
		// Enable I2C0 module clock. 
		CLK_EnableModuleClock(I2C1_MODULE);
		// Open I2C module and set bus clock. 
		I2C_Open(I2C1, MIC_I2C1_BUS_FREQ);
		// Enable I2C interrupt. 
		I2C_EnableInt(I2C1);
		NVIC_EnableIRQ(I2C1_IRQn);	
		// GPIO multi-function. (GPD14:I2C1_SCL, GPD15:I2C1_SDA) 
		SYS->GPD_MFPH = (SYS->GPD_MFPH & ~MIC_I2C1_PIN_MASK)|MIC_I2C1_PIN;
	}
	// (1-2) Send command to 85L40 via I2C1
	{
		uint16_t u16i;
		I2C_SetBusClockFreq(I2C1,MIC_I2C1_BUS_FREQ);
		s_MIC_I2CCtrl.u8DeviceAddr = MIC_I2C1_DEV_ADDR;
		for(u16i=0;u16i<sizeof(asMIC_Cmd_85L40)/sizeof(S_MIC_I2CCMD);u16i++) 
		{
			s_MIC_I2CCtrl.pau8Cmd = (uint8_t*)&asMIC_Cmd_85L40[u16i];
			s_MIC_I2CCtrl.u16Counter = 0;
			s_MIC_I2CCtrl.u16MaxCount = sizeof(S_MIC_I2CCMD);
			I2C_START(I2C1);
			/* Wait for I2C transmit completed. */
			while(s_MIC_I2CCtrl.u16MaxCount>0);
		}		
	}
}

void I2S_PDMA_Init(uint8_t ChannelNum)
{
	uint16_t pdma_len;
	
	// (2) Configure I2S to ge voice data from 85L40
	{
		// PD2: I2S0_MCLK, PD3: I2S0_LRCK, PD4: I2S0_DI, PD5: I2S0_DO, PD6: I2S0_BCLK
		SYS->GPD_MFPL = (SYS->GPD_MFPL & ~MIC_I2S0_PIN_MASK)|MIC_I2S0_PIN;
		// Enable I2S clock. 
		CLK_EnableModuleClock(I2S0_MODULE);
		// Select I2S clock. 	
		CLK_SetModuleClock(I2S0_MODULE, CLK_CLKSEL3_I2S0SEL_PLL, NULL);	
		
		// I2S IPReset. 
		SYS_ResetModule(I2S0_RST);
		// Open I2S and enable master clock. 

		switch ( ChannelNum )
		{
			case UAC_REC_CHANNEL_1CH:
			case UAC_REC_CHANNEL_2CH:
				I2S_Open(I2S0, I2S_MASTER, REC_RATE, I2S_DATABIT_16, I2S_TDMCHNUM_2CH, I2S_STEREO, I2S_FORMAT_PCMMSB); 
				pdma_len = AMIC2PDMA_BUFF_LEN_12CH;  
			break;
			
			case UAC_REC_CHANNEL_3CH:
			case UAC_REC_CHANNEL_4CH:
				I2S_Open(I2S0, I2S_MASTER, REC_RATE, I2S_DATABIT_16, I2S_TDMCHNUM_4CH, I2S_STEREO, I2S_FORMAT_PCMMSB);
				pdma_len = AMIC2PDMA_BUFF_LEN_34CH;  
			break;
		}

		// Enable I2C0 MCLK
		I2S_EnableMCLK(I2S0, MIC_I2S0_MCLK_FREQ);
		// I2S Configuration. 
		I2S_SET_PCMSYNC(I2S0, I2S_PCMSYNC_BCLK);
        
		// Set RX channel for Mono mode.
		if ( ChannelNum == 1 )
			I2S_SET_MONO_RX_CHANNEL(I2S0, I2S_MONO_RX_LEFT);
		else
			I2S_SET_MONO_RX_CHANNEL(I2S0, I2S_MONO_RX_RIGHT);
        
		// Set data order.
		I2S_SET_STEREOORDER(I2S0, I2S_ORDER_EVENLOW);
		// Enable I2S RXDMA
		I2S_ENABLE_RXDMA(I2S0);
		
		// Set channel width. 
#if (AMIC_BIT_RES == 16) 
		I2S_SET_CHWIDTH(I2S0, I2S_CHWIDTH_16);
#endif 
#if (AMIC_BIT_RES == 32) 
		I2S_SET_CHWIDTH(I2S0, I2S_CHWIDTH_32);
#endif
		// Set 16 bits data width.
		I2S_SET_PBWIDTH(I2S0, I2S_PBWIDTH_16);
		// Set FIFO Read/Write Order in 16-bit Width of Peripheral Bus.
		I2S_SET_PB16ORD(I2S0, I2S_PB16ORD_LOW);
		// Set FIFO threshold. 
		I2S_SET_RXTH(I2S0, I2S_FIFO_RX_LEVEL_WORD_9);
		// Clear TX, RX FIFO buffer 
		I2S_CLR_TX_FIFO(I2S0);
		I2S_CLR_RX_FIFO(I2S0);
	}
    
	// (3) Config PDMA for I2S transfer data.	
	{
		// Disable PDMA's NVIC
		NVIC_DisableIRQ(PDMA_IRQn);
        
#ifdef MIC_PDMA_16BIT
		/* Rx description */
		sPDMA_MIC[0].CTL = ((pdma_len /*AMIC2PDMA_BUFF_LEN*/ -1)<<PDMA_DSCT_CTL_TXCNT_Pos)|
																								PDMA_WIDTH_16		|
																								PDMA_SAR_FIX		|
																								PDMA_DAR_INC		|
																								PDMA_REQ_SINGLE	    |
																								PDMA_OP_SCATTER;
		sPDMA_MIC[0].SA = (uint32_t)&I2S0->RXFIFO;
		sPDMA_MIC[0].DA = (uint32_t)&UAC_REC.g_u32MICBuffer[0];
		sPDMA_MIC[0].NEXT = (uint32_t)&sPDMA_MIC[1] - (PDMA->SCATBA);

		sPDMA_MIC[1].CTL = ((pdma_len /*AMIC2PDMA_BUFF_LEN*/ -1)<<PDMA_DSCT_CTL_TXCNT_Pos)|
																								PDMA_WIDTH_16		|
																								PDMA_SAR_FIX		|
																								PDMA_DAR_INC		|
																								PDMA_REQ_SINGLE	    |
																								PDMA_OP_SCATTER;
		sPDMA_MIC[1].SA = (uint32_t)&I2S0->RXFIFO;
		sPDMA_MIC[1].DA = (uint32_t)&UAC_REC.g_u32MICBuffer[1];  
		sPDMA_MIC[1].NEXT = (uint32_t)&sPDMA_MIC[0] - (PDMA->SCATBA);   //link to first description	
#endif

		// Enable PDMA's NVIC
		NVIC_EnableIRQ(PDMA_IRQn);
	}
}

void I2C1_IRQHandler(void) 
{
	if(I2C_GET_TIMEOUT_FLAG(I2C1)) 
		I2C_ClearTimeoutFlag(I2C1); 
	else 
	{
		switch(I2C_GET_STATUS(I2C1)) 
		{
			// START has been transmitted and Write SLA+W to Register I2CDAT.
			case 0x08:
				I2C_SET_DATA(I2C1, s_MIC_I2CCtrl.u8DeviceAddr << 1);    
				I2C_SET_CONTROL_REG(I2C1, I2C_CTL_SI);			
			break;
			// SLA+W has been transmitted and ACK has been received.
			case 0x18:
				I2C_SET_DATA(I2C1, s_MIC_I2CCtrl.pau8Cmd[s_MIC_I2CCtrl.u16Counter++]);
				I2C_SET_CONTROL_REG(I2C1, I2C_CTL_SI);		
			break;
			// SLA+W has been transmitted and NACK has been received.
			case 0x20:
				I2C_STOP(I2C1);
				I2C_START(I2C1);	
				s_MIC_I2CCtrl.u16MaxCount = 0;
			break;
			// DATA has been transmitted and ACK has been received.
			case 0x28:
				if(s_MIC_I2CCtrl.u16Counter < s_MIC_I2CCtrl.u16MaxCount) 
				{
					I2C_SET_DATA(I2C1, s_MIC_I2CCtrl.pau8Cmd[s_MIC_I2CCtrl.u16Counter++]);
					I2C_SET_CONTROL_REG(I2C1, I2C_CTL_SI);
				} 
				else 
				{
					I2C_SET_CONTROL_REG(I2C1, I2C_CTL_STO_SI);
					// transfer complete
					s_MIC_I2CCtrl.u16MaxCount = 0;		
				}
			break;
		}
	}
}
