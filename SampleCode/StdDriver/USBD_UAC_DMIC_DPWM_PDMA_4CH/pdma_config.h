/**************************************************************************//**
 * @file     pdma_config.h
 * @version  V1.00
 * @brief    USB driver header file
 *
 * @copyright (C) 2017 Nuvoton Technology Corp. All rights reserved.
 ******************************************************************************/
#ifndef __PDMA_CONFIG_H__
#define __PDMA_CONFIG_H__

 #include "stdint.h" 

typedef struct pdma_config_t
{
	uint8_t  play_en;
	uint16_t play_buflen;
	uint8_t  rec_en;
	uint16_t rec_buflen;	
} PDMA_CONFIG_T;
extern PDMA_CONFIG_T PDMA_CONFIG;

typedef struct dma_desc_t 
{
    uint32_t ctl;
    uint32_t endsrc;
    uint32_t enddest;
    uint32_t offset;
} DMA_DESC_T;
extern DMA_DESC_T DMA_TXDESC[2], DMA_RXDESC[2];

#endif //__PDMA_CONFIG_H__

/*** (C) COPYRIGHT 2017 Nuvoton Technology Corp. ***/
