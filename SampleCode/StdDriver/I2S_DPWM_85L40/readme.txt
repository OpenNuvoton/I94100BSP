/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	Demonstrate how to implement a record and playback device by using NAU85L40 as microphones, DPWM to drive speakers and I2S to transfer data between I94100 and NAU85L40.
	(1)	Using I2C1 to set parameters to NAU85L40.
	(2)	Using I2S0 to receive audio data from NAU85L40.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect EVB-I94124ADI-NAU85L40B to JP4 on EVB-I94100.
	(2)	Connect speakers to JP4 and JP5 on EVB-I94124ADI-NAU85L40B.
	(3)	Compile and execute.
	(4)	Program test procedure ¡V
		1.	User can speak to MIC and speaker will playback simultaneously.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
	(1) If the actual HXT is not 12.288MHz on the board, please change the value of __HXT in "system_I94100.h" 
		and check the PLL setting accroding to "SYSCLK_PLL_CLK" description.
	(2) If the PLL and HIRC setting are changed in "ConfigSysClk.h", please check if the clock source of I2S
		is compliant with the sampling rate.