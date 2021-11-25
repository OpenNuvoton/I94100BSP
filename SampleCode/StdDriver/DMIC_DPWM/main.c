/******************************************************************************
 * @file     main.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 17/12/26 10:04a $
 * @brief	This sample uses DMIC as audio input(MIC) and DPWM as audio output(SPK) .
 *			User can process audio data before output.
 * @note
 * Copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
 
#include <stdio.h>
#include "Platform.h"
#include "BufCtrl.h"
#include "ConfigSysClk.h"

/*---------------------------------------------------------------------------------------------------------*/
/* Define global constants                                                                   			   */
/*---------------------------------------------------------------------------------------------------------*/
#define SAMPLE_RATE              (48000)

/*---------------------------------------------------------------------------------------------------------*/
/* Define functions prototype                                                                              */
/*---------------------------------------------------------------------------------------------------------*/
void SPK_Init(S_BUFCTRL* psOutBufCtrl);
void SPK_Start(void);
void SPK_Stop(void);
void MIC_Init(S_BUFCTRL* psInBufCtrl);
void MIC_Start(void);
void MIC_Stop(void);

/*---------------------------------------------------------------------------------------------------------*/
/* Define global variables and constants                                                                   */
/*---------------------------------------------------------------------------------------------------------*/
volatile S_BUFCTRL sInBufCtrl,sOutBufCtrl;// Buffer control handler.
int32_t    ai32InBuf[256];  	          // Buffer array: store audio data receiced from DMIC
int32_t    ai32OutBuf[128]; 	          // Buffer array: store audio data ready to send to DPWM

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int main()
{
	int32_t i32Data[4];
	
	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	// These defines are from  BUFCTRL.h for buffer control in this samle. 
	// Buffer control handler configuration. 
	BUFCTRL_CFG((&sInBufCtrl),ai32InBuf,sizeof(ai32InBuf)/sizeof(uint32_t));
	BUFCTRL_CFG((&sOutBufCtrl),ai32OutBuf,sizeof(ai32OutBuf)/sizeof(uint32_t));
	// full empty data into output buffer.
	sOutBufCtrl.u16DataCount = sOutBufCtrl.u16BufCount;  
	
	// Initiate microphone.
	MIC_Init((S_BUFCTRL*)&sInBufCtrl);
	// Initiate speaker.
	SPK_Init((S_BUFCTRL*)&sOutBufCtrl);
	
	// Start microphone.
	MIC_Start();
	// Start speaker.
	SPK_Start();
		
	// while loop for processing.
	while(1) 
	{
		while( BUFCTRL_GET_COUNT((&sInBufCtrl))>= 4 && !BUFCTRL_IS_FULL((&sOutBufCtrl)) )
		{
			// 4 channel mixer to 2 channe
			BUFCTRL_READ((&sInBufCtrl),&i32Data[0]);
			BUFCTRL_READ((&sInBufCtrl),&i32Data[1]);
			BUFCTRL_READ((&sInBufCtrl),&i32Data[2]);
			BUFCTRL_READ((&sInBufCtrl),&i32Data[3]);	
			i32Data[0] = i32Data[0]+i32Data[2];
			i32Data[1] = i32Data[1]+i32Data[3];
			BUFCTRL_WRITE((&sOutBufCtrl),i32Data[0]);
			BUFCTRL_WRITE((&sOutBufCtrl),i32Data[1]);
		}
	};
}

// Microphone(DMIC)= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
#define MIC_PIN_MASK    (SYS_GPD_MFPL_PD3MFP_Msk|SYS_GPD_MFPL_PD4MFP_Msk|SYS_GPD_MFPL_PD5MFP_Msk|SYS_GPD_MFPL_PD6MFP_Msk)
#define MIC_PIN         (SYS_GPD_MFPL_PD3MFP_DMIC_CLK1|SYS_GPD_MFPL_PD4MFP_DMIC_DAT1|SYS_GPD_MFPL_PD5MFP_DMIC_CLK0|SYS_GPD_MFPL_PD6MFP_DMIC_DAT0)

S_BUFCTRL* psMIC_BufCtrl = NULL;           // Provide microphone input buffer control.

void MIC_Init(S_BUFCTRL* psInBufCtrl)
{
	// Select DMIC CLK source from PLL. 
	CLK_SetModuleClock(DMIC_MODULE, CLK_CLKSEL2_DMICSEL_PCLK0, MODULE_NoMsk);	
	// Enable DMIC clock. 
	CLK_EnableModuleClock(DMIC_MODULE);
	// DPWM IPReset. 
	SYS_ResetModule(DMIC_RST);
	// Set down sample rate 100 for quilty.(Suggest 96M used DMIC_CTL_DOWNSAMPLE_100_50 )
	DMIC_ENABLE_DOWMSAMPLE(DMIC,DMIC_CTL_DOWNSAMPLE_100_50);
	// Set DMIC sample rate.
	DMIC_SetSampleRate(DMIC,SAMPLE_RATE);
	// Set channel's latch data falling type. 
	DMIC_SET_LATCHDATA(DMIC,DMIC_CTL_LATCHDATA_CH01F|DMIC_CTL_LATCHDATA_CH23F);
	// Enable DMIC FIFO threshold interrupt.
	DMIC_ENABLE_FIFOTHRESHOLDINT(DMIC,8);
	// Enable DMIC NVIC interrupt.
	NVIC_EnableIRQ(DMIC_IRQn);	
	// GPIO multi-function.	
	SYS->GPD_MFPL = (SYS->GPD_MFPL & ~MIC_PIN_MASK) | MIC_PIN;
	// Config DPWM(Speaker) buffer control 
	psMIC_BufCtrl = psInBufCtrl;
}
void MIC_Start(void)
{
	if( psMIC_BufCtrl != NULL )
	{
		DMIC_ENABLE_CHANNEL(DMIC,DMIC_CTL_CH0|DMIC_CTL_CH1|DMIC_CTL_CH2|DMIC_CTL_CH3);
	}
}
void MIC_Stop(void)
{
	DMIC_DISABLE_CHANNEL(DMIC,DMIC_CTL_CH0|DMIC_CTL_CH1|DMIC_CTL_CH2|DMIC_CTL_CH3);
}
void DMIC_IRQHandler() 
{
	uint32_t u32Tmp, i; 
	
	for(i=0; i<8; i++)
    {
		if(!DMIC_IS_FIFOEMPTY(DMIC))
		{
			// Read the data from I2S RXFIFO, but not write to bufer. 
			u32Tmp = DMIC_READ_DATA(DMIC);
			
			// Write the data from I2S RXFIFO to buffer.
			if(!BUFCTRL_IS_FULL(psMIC_BufCtrl) && psMIC_BufCtrl != NULL)
				BUFCTRL_WRITE(psMIC_BufCtrl,u32Tmp);
		}
		else
			break;
	}
}
// Speaker(DPWM) = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
#define SPK_PIN_MASK    (SYS_GPC_MFPH_PC13MFP_Msk|SYS_GPC_MFPH_PC12MFP_Msk|SYS_GPC_MFPH_PC11MFP_Msk|SYS_GPC_MFPH_PC10MFP_Msk)
#define SPK_PIN         (SYS_GPC_MFPH_PC13MFP_DPWM_LP|SYS_GPC_MFPH_PC12MFP_DPWM_LN|SYS_GPC_MFPH_PC11MFP_DPWM_RP|SYS_GPC_MFPH_PC10MFP_DPWM_RN)

S_BUFCTRL* psSPK_BufCtrl = NULL;            // Provide Speaker to output data.

void SPK_Init(S_BUFCTRL* psOutBufCtrl)
{
	// Select DPWM CLK source from PLL. 
	CLK_SetModuleClock(DPWM_MODULE, CLK_CLKSEL2_DPWMSEL_PCLK0, MODULE_NoMsk);	
	// Enable DPWM clock. 
	CLK_EnableModuleClock(DPWM_MODULE);
	// DPWM IPReset. 
	SYS_ResetModule(DPWM_RST);
	// Set clock frequency.
	DPWM_SET_CLKSET(DPWM, DPWM_CLKSET_500FS);
	// Set DPWM output sample rate.
	DPWM_SetSampleRate(SAMPLE_RATE);
	// Set fifo data width 24Bits. 
	DPWM_SET_FIFODATAWIDTH(DPWM, DPWM_FIFO_DATAWIDTH_24BITS);
	// Enable threshold int 
	DPWM_ENABLE_FIFOTHRESHOLDINT(DPWM,8);
	// Enable NVIC.
	NVIC_EnableIRQ(DPWM_IRQn);
	// GPIO multi-function.
    SYS->GPC_MFPH = (SYS->GPC_MFPH & ~SPK_PIN_MASK)| SPK_PIN;		
	// Config DPWM(Speaker) buffer control 
	psSPK_BufCtrl = psOutBufCtrl;
}
void SPK_Start(void)
{
	if( psSPK_BufCtrl != NULL )
	{
		DPWM_ENABLE_DRIVER(DPWM);
		DPWM_START_PLAY(DPWM);
	}
}
void SPK_Stop(void)
{
	DPWM_STOP_PLAY(DPWM);
	DPWM_DISABLE_DRIVER(DPWM);	
}
void DPWM_IRQHandler(void) 
{
	int32_t i32Tmp, i32i;
	
	if( BUFCTRL_IS_EMPTY(psSPK_BufCtrl) ) 
	{
		if( DPWM_IS_FIFOEMPTY(DPWM) )
		{
			for( i32i=0; i32i<8; i32i++ )
				DPWM_WRITE_INDATA(DPWM,0);
		}
	} 
	else 
	{
		for(i32i=0; i32i<4; i32i++)
		{
			if(!DPWM_IS_FIFOFULL(DPWM)) 
			{
				if( !BUFCTRL_IS_EMPTY(psSPK_BufCtrl) )
				{
					BUFCTRL_READ(psSPK_BufCtrl,&i32Tmp);
					DPWM_WRITE_INDATA(DPWM,i32Tmp);
				}
			}
			else
				break;
		}
	}
}
