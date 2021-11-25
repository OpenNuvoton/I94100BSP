/**************************************************************************//**
 * @file     SysClk.c
 * @version  V1.00
 * $Revision: 1 $
 * $Date: 19/11/22 18:10p $
 * @brief    System clock configuration file for framework.
 * @note
 * Copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/
#include "ConfigSysClk.h"
#include "SysClk.h"
	
// Initiate system clock depend on configuration in ConfigSysClk.h
UINT32 SysClk_Initiate(void)
{
	// Unlock protected registers
	uint8_t u8Lock = SYS_Unlock();
	
	// 1. Configuration HIRC ====================================================
#if (SYSCLK_HIRC_CLK==SYSCLK_HIRC_CLK_49152)
	// Select trim HIRC Frequency to 49.152MHz
	CLK_SELECT_TRIM_HIRC(CLK_CLKSEL0_HIRCFSEL_49M);
#else
	// Select trim HIRC Frequency to 48MHz
	CLK_SELECT_TRIM_HIRC(CLK_CLKSEL0_HIRCFSEL_48M);
#endif
	// Enable HIRC 
	CLK_EnableXtalRC(CLK_PWRCTL_HIRCEN_Msk);
	// Waiting for HIRC clock ready 
	CLK_WaitClockReady(CLK_STATUS_HIRCSTB_Msk);

	// 2. Configuration HXT(if enable HXT) ======================================
#if ((SYSCLK_HCLK_CLK == SYSCLK_HCLK_CLK_PLL_HXT) || (SYSCLK_HCLK_CLK == SYSCLK_HCLK_CLK_HXT))
    // Enable HXT 
    CLK_EnableXtalRC(CLK_PWRCTL_HXTEN_Msk);
    // Waiting for HIRC clock ready 
    CLK_WaitClockReady(CLK_STATUS_HXTSTB_Msk);
#endif	

	// 3. Configuration HCLK(if source clock is PLL, enable PLL first) ==========
#if ((SYSCLK_HCLK_CLK==SYSCLK_HCLK_CLK_PLL_HIRC)||(SYSCLK_HCLK_CLK==SYSCLK_HCLK_CLK_PLL_HXT))
	// Enable PLL clock
	CLK_EnablePLL(((SYSCLK_HCLK_CLK==SYSCLK_HCLK_CLK_PLL_HIRC)?CLK_PLLCTL_PLLSRC_HIRC:CLK_PLLCTL_PLLSRC_HXT), (SYSCLK_PLL_CLK*1000));
	// Switch HCLK clock source to PLL
	CLK_SetHCLK(CLK_CLKSEL0_HCLKSEL_PLL, CLK_CLKDIV0_HCLK(1));
	
	#if (SYSCLK_PLL_CLK > 90000UL)
	CLK_SetPCLKDivider(CLK_PCLKDIV_PCLK0DIV2);
	CLK_SetPCLKDivider(CLK_PCLKDIV_PCLK1DIV2);
	#endif
	
#else
	// Set HCLK clock source to HIRC(SYSCLK_HCLK_CLK_HIRC) or HXT(SYSCLK_HCLK_CLK_HXT)
	CLK_SetHCLK(((SYSCLK_HCLK_CLK==SYSCLK_HCLK_CLK_HIRC)?CLK_CLKSEL0_HCLKSEL_HIRC:CLK_CLKSEL0_HCLKSEL_HXT), CLK_CLKDIV0_HCLK(1));
#endif

	// Update System Core Clock. 
	// User can use SystemCoreClockUpdate() to calculate PllClock, SystemCoreClock and CyclesPerUs automatically.
	SystemCoreClockUpdate();
	// Lock protected registers. 
	SYS_Lock(u8Lock);	

	return CLK_GetHCLKFreq();
}

/*** (C) COPYRIGHT 2018 Nuvoton Technology Corp. ***/
