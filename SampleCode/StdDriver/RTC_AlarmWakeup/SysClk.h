#ifndef __SYSCLK_H__
#define __SYSCLK_H__

#include "Platform.h"

#ifdef __cplusplus
extern "C"
{
#endif
	
/**
  * @brief   Provide user selections of HIRC's frequency.
  * @details 
  * SYSCLK_HIRC_CLK_49152 : HIRC's frequency is 49.152 MHz.
  * SYSCLK_HIRC_CLK_48000 : HIRC's frequency is 48 MHz.
  */
#define SYSCLK_HIRC_CLK_49152    (0)   
#define SYSCLK_HIRC_CLK_48000    (1)   

/**
  * @brief   Provide user selections of HCLK's clock source.
  * @details 
  * SYSCLK_HCLK_CLK_HIRC     : HCLK's clock source is from HIRC.
  * SYSCLK_HCLK_CLK_HXT      : HCLK's clock source is from HXT.
  * SYSCLK_HCLK_CLK_PLL_HIRC : HCLK's clock source is from PLL, and PLL's clock source is from HIRC.
  * SYSCLK_HCLK_CLK_PLL_HXT  : HCLK's clock source is from PLL, and PLL's clock source is from HXT.
  */
#define SYSCLK_HCLK_CLK_HIRC     (0)    
#define SYSCLK_HCLK_CLK_HXT      (1)    
#define SYSCLK_HCLK_CLK_PLL_HIRC (2)     
#define SYSCLK_HCLK_CLK_PLL_HXT  (3)    
	
/**
  * @brief  This function generates desired system clock via the configuration defined in ConfigSysClk.h.
  * @return The actually system clock
  */
UINT32 SysClk_Initiate(void);

#ifdef __cplusplus
}
#endif

#endif
