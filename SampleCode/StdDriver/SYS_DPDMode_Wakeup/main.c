/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * @brief
 *           Show how to wake up system form DPD Power-down mode by Wake-up pin(PA.15)
 *           or Wake-up Timer.  
 *
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "stdio.h"
#include "Platform.h"
#include "ConfigSysClk.h"

extern int IsDebugFifoEmpty(void);

/*---------------------------------------------------------------------------------------------------------*/
/*  Function for System Entry to Power Down Mode and Wake up source by Wake-up pin                         */
/*---------------------------------------------------------------------------------------------------------*/
void WakeUpPinFunction(uint32_t u32PDMode, uint32_t u32EdgeType)
{
    printf("Enter to DPD Power-Down mode......\n");
	while(IsDebugFifoEmpty() == 0);
	UART_Close(UART0);
	CLK_DisableModuleClock(UART0_MODULE);
	
	CLK_DisableXtalRC(CLK_PWRCTL_LXTEN_Msk | CLK_PWRCTL_LIRCEN_Msk);
	
	// Set I/O pins as GPIO and Quasi mode
	SYS->GPA_MFPL = 0x00000000 ; // all set GPIO mode 
	SYS->GPA_MFPH = 0x00000000 ; //all set GPIO mode
	SYS->GPB_MFPL = 0x00000000 ; // all set GPIO mode
	SYS->GPB_MFPH = 0x00000000 ; // all set GPIO mode
    SYS->GPC_MFPL = 0x00000000 ; // all set GPIO mode 
	SYS->GPC_MFPH = 0x00000000 ; // all set GPIO mode	
	SYS->GPD_MFPL = 0x00000000 ; // all set GPIO mode 
	SYS->GPD_MFPH = 0x00000000 ; // all set GPIO mode 
	
	GPIO_SET_OUT_DATA(PA, 0xFFFFFFFF);
	GPIO_SET_OUT_DATA(PB, 0xFFFFFFFF);
	GPIO_SET_OUT_DATA(PC, 0xFFFFFFFF);
	GPIO_SET_OUT_DATA(PD, 0xFFFFFFFF);
	
	PA->MODE = 0xFFFFFFFF;  // set PA all Quasi mode
	PB->MODE = 0xFFFFFFFF;  // set PB all Quasi mode
	PC->MODE = 0xFFFFFFFF ; // set PC all Quasi
	PD->MODE = 0xFFFFFFFF ; // set PD all Quasi
	
    // Set Wake-up pin trigger type at Deep Power down mode
	// Note: Wake-up pin needs to add the external pull high resistor, or wake-up function trigger easily
	CLK_ENABLE_DPDWKPIN(u32EdgeType);
	
	 /* Select Power-down mode */
    CLK_SetPowerDownMode(u32PDMode);
	
    /* Enter to Power-down mode */
    CLK_PowerDown();

    /* Wait for Power-down mode wake-up reset happen */
    while(1);
}

/*-----------------------------------------------------------------------------------------------------------*/
/*  Function for System Entry to Power Down Mode and Wake up source by Wake-up Timer                         */
/*-----------------------------------------------------------------------------------------------------------*/
void  WakeUpTimerFunction(uint32_t u32PDMode, uint32_t u32Interval)
{

    printf("Enter to DPD Power-Down mode......\n");
	while(IsDebugFifoEmpty() == 0);
	UART_Close(UART0);
	CLK_DisableModuleClock(UART0_MODULE);
	
	CLK_DisableXtalRC(CLK_PWRCTL_LXTEN_Msk);
	
	// Set I/O pins as GPIO and Quasi mode
	SYS->GPA_MFPL = 0x00000000 ; // all set GPIO mode 
	SYS->GPA_MFPH = 0x00000000 ; //all set GPIO mode
	SYS->GPB_MFPL = 0x00000000 ; // all set GPIO mode
	SYS->GPB_MFPH = 0x00000000 ; // all set GPIO mode
    SYS->GPC_MFPL = 0x00000000 ; // all set GPIO mode 
	SYS->GPC_MFPH = 0x00000000 ; // all set GPIO mode	
	SYS->GPD_MFPL = 0x00000000 ; // all set GPIO mode 
	SYS->GPD_MFPH = 0x00000000 ; // all set GPIO mode 
	
	GPIO_SET_OUT_DATA(PA, 0xFFFFFFFF);
	GPIO_SET_OUT_DATA(PB, 0xFFFFFFFF);
	GPIO_SET_OUT_DATA(PC, 0xFFFFFFFF);
	GPIO_SET_OUT_DATA(PD, 0xFFFFFFFF);
	
	PA->MODE = 0xFFFFFFFF;  // set PA all Quasi mode
	PB->MODE = 0xFFFFFFFF;  // set PB all Quasi mode
	PC->MODE = 0xFFFFFFFF ; // set PC all Quasi
	PD->MODE = 0xFFFFFFFF ; // set PD all Quasi
	
    /* Set Wake-up Timer Time-out Interval */
    CLK_SET_WKTMR_INTERVAL(u32Interval);

    /* Enable Wake-up Timer */
    CLK_ENABLE_WKTMR();
	
	/* Select Power-down mode */
    CLK_SetPowerDownMode(u32PDMode);
	
    /* Enter to Power-down mode */
    CLK_PowerDown();

    /* Wait for Power-down mode wake-up reset happen */
    while(1);
}

/*-----------------------------------------------------------------------------------------------------------*/
/*  Function for Check Power Manager Status                                                                  */
/*-----------------------------------------------------------------------------------------------------------*/
void CheckPowerSource(void)
{
    unsigned int uRegRstsrc;
    uRegRstsrc = CLK_GetPMUWKSrc();

    printf("Power manager Power Manager Status 0x%x\n", uRegRstsrc);
	
	if((uRegRstsrc & CLK_PMUSTS_DPD_RSTWK_Msk) != 0)
        printf("Wake-up source is RESET pin.\n");
    if((uRegRstsrc & CLK_PMUSTS_DPD_TMRWK_Msk) != 0)
        printf("Wake-up source is Wake-up Timer.\n");
    if((uRegRstsrc & CLK_PMUSTS_PINWK_Msk) != 0)
        printf("Wake-up source is Wake-up Pin.\n");
}

void UART0_Init()
{
    /* Enable peripheral clock */
    CLK_EnableModuleClock(UART0_MODULE);
    /* Select UART module clock source as PLL and UART module clock divider as 1 */
    CLK_SetModuleClock(UART0_MODULE, CLK_CLKSEL1_UART0SEL_PLL, CLK_CLKDIV0_UART0(1));
    /* Set PB multi-function pins for UART0 RXD(PB.9) and TXD(PB.8) */
    SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB8MFP_Msk | SYS_GPB_MFPH_PB9MFP_Msk);
    SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB8MFP_UART0_TXD | SYS_GPB_MFPH_PB9MFP_UART0_RXD);
    /* Reset UART0 */
    SYS_ResetModule(UART0_RST);
    /* Configure UART0 and set UART0 baud rate */
    UART_Open(UART0, 115200);
}

/*---------------------------------------------------------------------------------------------------------*/
/*  Main Function                                                                                          */
/*---------------------------------------------------------------------------------------------------------*/
int32_t main(void)
{
	uint8_t u8Lock, u8Item;

	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	/* Init UART0 for printf */
	UART0_Init();

	u8Lock = SYS_Unlock();
	/* Enable LIRC */
	CLK_EnableXtalRC(CLK_PWRCTL_LIRCEN_Msk);
	/* Waiting for clock ready */
	CLK_WaitClockReady(CLK_STATUS_LIRCSTB_Msk);
	SYS_Lock(u8Lock);

	/* Set core clock as PLL_CLOCK from PLL and SysTick source to HCLK/2*/
	CLK_SetSysTickClockSrc(CLK_CLKSEL0_STCLKSEL_HCLK_DIV2);

	
	printf("\n\nCPU @ %d Hz\n", SystemCoreClock);

	/* Get power manager wake up source */
	CheckPowerSource();

	printf("+----------------------------------------------------------------+\n");
	printf("|     DPD Power-down Mode and Wake-up Sample Code.               |\n");
	printf("|    Please Select Wake up source.                               |\n");
	printf("+----------------------------------------------------------------+\n");
	printf("|[1] DPD Wake-up Pin(PA.15) trigger type is rising edge.         |\n");
	printf("|[2] DPD Wake-up TIMER time-out interval is 1024 OSC10K clocks.  |\n");
	printf("+----------------------------------------------------------------+\n");
	u8Item = getchar();

	/* Unlock protected registers */
	SYS_UnlockReg();
	
	// Disable Brown-Out Detector Control
	SYS->BODCTL=0x00050000;
	// Disable LDO Control
	CLK->LDOCTL = 0;
	
    switch(u8Item) {
    case '1':
        WakeUpPinFunction(CLK_PMUCTL_PDMSEL_DPD, CLK_DPDWKPIN_RISING);
        break;
    case '2':
        WakeUpTimerFunction(CLK_PMUCTL_PDMSEL_DPD, CLK_PMUCTL_WKTMRIS_1024);
        break;
    default:
        break;
    }
    while(1);
}

