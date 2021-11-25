/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	This sample code shows how to implement a USB recording device using codec NAU85L40.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect EVB-I94124ADI-NAU85L40B to JP4 on EVB-I94100.
	(2)	Compile and execute.
	(3)	Connect USB to USB port on the EVB (CN1). 
	(4)	Program test procedure ¡V
		1.	PC will recognize the device as USB recording device.
		2.	On PC, user should change the recording device to ¡§Microphone (USB Device)¡¨. 
		3.	User could use any recording software to test the demo code.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
	(1) HIRC Auto Trim:
		1. The sample code uses HIRC auto trim function to stabilize the HIRC frequency.
		2. TIMER0 handler will enable HIRC auto trim for every 10 second.
	(2) If the actual HXT is not 12.288MHz on the board, please change the value of __HXT in "system_I94100.h" 
		and check the PLL setting accroding to "SYSCLK_PLL_CLK" description.
	(3) If the PLL and HIRC setting are changed in "ConfigSysClk.h", please check if the clock source of I2S
		is compliant with the sampling rate.