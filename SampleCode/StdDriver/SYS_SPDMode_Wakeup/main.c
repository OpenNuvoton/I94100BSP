/**************************************************************************//**
 * @file     main.c
 * @version  V3.00
 * @brief
 *           Show how to wake up system form SPD Power-down mode by GPIO pin(PA.0)
 *           or Wake-up Timer.
 *
 *
 * @copyright (C) 2016 Nuvoton Technology Corp. All rights reserved.
 *
 ******************************************************************************/
#include "stdio.h"
#include "Platform.h"
#include "ConfigSysClk.h"

#define SIGNATURE  0x125ab234
#define FLAG_ADDR  0x20001FFF

extern int IsDebugFifoEmpty(void);

/*---------------------------------------------------------------------------------------------------------*/
/*  Function for System Entry to Power Down Mode and Wake up source by GPIO Wake-up pin                    */
/*---------------------------------------------------------------------------------------------------------*/
void WakeUpPinFunction(uint32_t u32PDMode, uint32_t u32EdgeType)
{
	printf("Enter to SPD Power-Down mode......\n");
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
	  
	// GPIO SPD Power-down GPA15 Pin Select and Debounce Enable
    CLK_EnableSPDWKPin(0, 15, CLK_SPDWKPIN_RISING, CLK_SPDWKPIN_DEBOUNCEEN);
	
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
	printf("Enter to SPD Power-Down mode......\n");
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
	
    if((uRegRstsrc & CLK_PMUSTS_SPD_TMRWK_Msk) != 0)
        printf("Wake-up source is SPD mode Wake-up Timer.\n");
    if((uRegRstsrc & CLK_PMUSTS_GPAWK_Msk) != 0)
        printf("Wake-up source is GPA0.\n");

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
	uint8_t u8Item;

	// Initiate system clock(Configure in ConfigSysClk.h)
	SYSCLK_INITIATE();

	// Set system tick clock.
	CLK_SetSysTickClockSrc(CLK_CLKSEL0_STCLKSEL_HCLK_DIV2);

	/* Init UART0 for printf */
	UART0_Init();
	
	/* Unlock protected registers */
	SYS_UnlockReg();

	/* Release I/O hold status */
	CLK->IOPDCTL = 1;

	printf("\n\nCPU @ %d Hz\n\n", SystemCoreClock);

	/* Get power manager wake up source */
	CheckPowerSource();

	if(M32(FLAG_ADDR) == SIGNATURE) {
		printf("System waken-up from SPD0 mode done!\n");
	M32(FLAG_ADDR) = 0;
		printf("Press any key to continue ...\n");
		getchar();
	} else
		printf("System waken-up from SPD1 mode done!\n");

	printf("+-----------------------------------------------------------------+\n");
	printf("|     SPD Power-down Mode and Wake-up Sample Code                 |\n");
	printf("|    Please Select Power Down Mode and Wake up source.            |\n");
	printf("+-----------------------------------------------------------------+\n");
	printf("|[0] SPD0 GPIO pin(PA.15) and using rising edge wake up.           |\n");
	printf("|[1] SPD0 Wake-up TIMER time-out interval is 1024 OSC10K clocks.  |\n");
	printf("|[2] SPD1 GPIO pin(PA.15) and using rising edge wake up.           |\n");
	printf("|[3] SPD1 Wake-up TIMER time-out interval is 1024 OSC10K clocks.  |\n");
	printf("+-----------------------------------------------------------------+\n");
	u8Item = getchar();

	// SRAM retention test
	M32(FLAG_ADDR) = SIGNATURE;

	// Disable Brown-Out Detector Control
	SYS->BODCTL=0x00050000;
	// Disable LDO Control
	CLK->LDOCTL = 0;
	
	switch(u8Item) {
    case '0':
        WakeUpPinFunction(CLK_PMUCTL_PDMSEL_SPD0, CLK_DPDWKPIN_RISING);
        break;
    case '1':
        WakeUpTimerFunction(CLK_PMUCTL_PDMSEL_SPD0, CLK_PMUCTL_WKTMRIS_1024);
        break;
    case '2':
        WakeUpPinFunction(CLK_PMUCTL_PDMSEL_SPD1, CLK_DPDWKPIN_RISING);
        break;
    case '3':
        WakeUpTimerFunction(CLK_PMUCTL_PDMSEL_SPD1, CLK_PMUCTL_WKTMRIS_1024);
        break;
    default:
        break;
    }

    while(1);
}
