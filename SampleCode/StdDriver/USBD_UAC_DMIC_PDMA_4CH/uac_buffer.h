/**************************************************************************//**
 * @file     uac_buffer.h
 * @version  V1.00
 * @brief    USB driver header file
 *
 * @copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef __UAC_BUFFER_H__
#define __UAC_BUFFER_H__

#include "stdint.h" 
#include "usbd_audio.h"

// Audio Parameter
// volatile uint8_t g_usbd_PlayMute      = 0x00;     /* Play MUTE control. 0 = normal. 1 = MUTE */
// volatile int16_t g_usbd_PlayVolumeL   = 0x1000;   /* Play left channel volume. Range is -32768 ~ 32767 */
// volatile int16_t g_usbd_PlayVolumeR   = 0x1000;   /* Play right channel volume. Range is -32768 ~ 32767 */
// volatile int16_t g_usbd_PlayMaxVolume = 0x7FFF;
// volatile int16_t g_usbd_PlayMinVolume = 0x8000;
// volatile int16_t g_usbd_PlayResVolume = 0x400;

// volatile uint8_t g_usbd_Rec1Mute       = 0x01;     /* Record MUTE control. 0 = normal. 1 = MUTE */
// volatile int16_t g_usbd_Rec1VolumeL    = 0x1000;   /* Record left channel volume. Range is -32768 ~ 32767 */
// volatile int16_t g_usbd_Rec1VolumeR    = 0x1000;   /* Record right channel volume. Range is -32768 ~ 32767 */
// volatile int16_t g_usbd_Rec1MaxVolume  = 0x7FFF;
// volatile int16_t g_usbd_Rec1MinVolume  = 0x8000;
// volatile int16_t g_usbd_Rec1ResVolume  = 0x400;

// volatile uint8_t g_usbd_Rec2Mute       = 0x01;     /* Record MUTE control. 0 = normal. 1 = MUTE */
// volatile int16_t g_usbd_Rec2VolumeL    = 0x1000;   /* Record left channel volume. Range is -32768 ~ 32767 */
// volatile int16_t g_usbd_Rec2VolumeR    = 0x1000;   /* Record right channel volume. Range is -32768 ~ 32767 */
// volatile int16_t g_usbd_Rec2MaxVolume  = 0x7FFF;
// volatile int16_t g_usbd_Rec2MinVolume  = 0x8000;
// volatile int16_t g_usbd_Rec2ResVolume  = 0x400;

// volatile uint8_t g_usbd_Rec3Mute       = 0x01;     /* Record MUTE control. 0 = normal. 1 = MUTE */
// volatile int16_t g_usbd_Rec3VolumeL    = 0x1000;   /* Record left channel volume. Range is -32768 ~ 32767 */
// volatile int16_t g_usbd_Rec3VolumeR    = 0x1000;   /* Record right channel volume. Range is -32768 ~ 32767 */
// volatile int16_t g_usbd_Rec3MaxVolume  = 0x7FFF;
// volatile int16_t g_usbd_Rec3MinVolume  = 0x8000;
// volatile int16_t g_usbd_Rec3ResVolume  = 0x400;

// volatile uint8_t g_usbd_Rec4Mute       = 0x01;     /* Record MUTE control. 0 = normal. 1 = MUTE */
// volatile int16_t g_usbd_Rec4VolumeL    = 0x1000;   /* Record left channel volume. Range is -32768 ~ 32767 */
// volatile int16_t g_usbd_Rec4VolumeR    = 0x1000;   /* Record right channel volume. Range is -32768 ~ 32767 */
// volatile int16_t g_usbd_Rec4MaxVolume  = 0x7FFF;
// volatile int16_t g_usbd_Rec4MinVolume  = 0x8000;
// volatile int16_t g_usbd_Rec4ResVolume  = 0x400;

typedef struct _uac_playback_t
{
   uint8_t  u8AudioSpeakerState;  
   uint32_t usbd_PlaySampleRate;
   uint32_t au32PDMA2DPWM_Buff[2][96]; 
   uint32_t au32USB2PDMA_Buff[MAX_USB_BUFFER_LEN];  
   uint16_t u16PDMA2DPWM_Bufflen; 
   uint16_t u16PlayBack_Read_Ptr; 
   uint16_t u16PlayBack_Write_Ptr; 
   uint16_t u16PlayBack_Ptrs_Distance; 
   uint8_t  u8DPWM_Buff_Index;   
   uint8_t  u8Update_USB_Buffer_Flag;  
    
   // parameters
   uint8_t  usbd_PlayMute;       /* Play MUTE control. 0 = normal. 1 = MUTE */
   int16_t  usbd_PlayVolumeL;    /* Play left channel volume. Range is -32768 ~ 32767 */
   int16_t  usbd_PlayVolumeR;    /* Play right channel volume. Range is -32768 ~ 32767 */
   int16_t  usbd_PlayMaxVolume; 
   int16_t  usbd_PlayMinVolume; 
   int16_t  usbd_PlayResVolume; 
        
} UAC_PLAYBACK_T;
extern UAC_PLAYBACK_T UAC_PLAYBACK;


typedef struct _uac_recoder_t
{
  uint8_t  u8AudioMicState;
  uint8_t  u8AudioRecInterfConfig;
  uint32_t au32DMIC2PDMA_Buff[2][DMIC2PDMA_BUFF_LEN];
  uint32_t u8DMIC_Buff_Index;
  uint16_t u16UAC_Buff_ReadIndex;
  uint16_t u16UAC_Buff_WriteIndex;
    
  // parameters
  // REC_1CH
  uint8_t usbd_Rec1Mute;       /* Record MUTE control. 0 = normal. 1 = MUTE */
  int16_t usbd_Rec1VolumeL;    /* Record left channel volume. Range is -32768 ~ 32767 */
  int16_t usbd_Rec1VolumeR;    /* Record right channel volume. Range is -32768 ~ 32767 */
  int16_t usbd_Rec1MaxVolume;  
  int16_t usbd_Rec1MinVolume;  
  int16_t usbd_Rec1ResVolume;  
  // REC_2CH
  uint8_t usbd_Rec2Mute;       /* Record MUTE control. 0 = normal. 1 = MUTE */
  int16_t usbd_Rec2VolumeL;    /* Record left channel volume. Range is -32768 ~ 32767 */
  int16_t usbd_Rec2VolumeR;    /* Record right channel volume. Range is -32768 ~ 32767 */
  int16_t usbd_Rec2MaxVolume;  
  int16_t usbd_Rec2MinVolume;  
  int16_t usbd_Rec2ResVolume;  
  // REC_3CH
  uint8_t usbd_Rec3Mute;       /* Record MUTE control. 0 = normal. 1 = MUTE */
  int16_t usbd_Rec3VolumeL;    /* Record left channel volume. Range is -32768 ~ 32767 */
  int16_t usbd_Rec3VolumeR;    /* Record right channel volume. Range is -32768 ~ 32767 */
  int16_t usbd_Rec3MaxVolume;  
  int16_t usbd_Rec3MinVolume;  
  int16_t usbd_Rec3ResVolume;  
  // REC_4CH 
  uint8_t usbd_Rec4Mute;       /* Record MUTE control. 0 = normal. 1 = MUTE */
  int16_t usbd_Rec4VolumeL;    /* Record left channel volume. Range is -32768 ~ 32767 */
  int16_t usbd_Rec4VolumeR;    /* Record right channel volume. Range is -32768 ~ 32767 */
  int16_t usbd_Rec4MaxVolume;  
  int16_t usbd_Rec4MinVolume;  
  int16_t usbd_Rec4ResVolume;      
} UAC_RECODER_T;
extern UAC_RECODER_T UAC_RECODER;


#endif //__UAC_BUFFER_H__

/*** (C) COPYRIGHT 2018 Nuvoton Technology Corp. ***/


  
