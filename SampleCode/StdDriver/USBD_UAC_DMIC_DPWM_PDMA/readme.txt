/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	This sample code shows how to implement a USB recording and playback device using DMIC and DPWM.
	(1)	Using EVB-I94124ADI-NAU85L40B (DMIC version) to demo.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect EVB-I94124ADI-NAU85L40B to JP4 on EVB-I94100.
	(2)	Connect speaker to DPWM_R on JP5 on EVB.
	(3)	Compile and execute.
	(4)	Connect USB to USB port on the EVB (CN1). 
	(5)	Program test procedure ¡V
		1.	PC will recognize the device as USB recording and also playback device.
		2.	On PC, user should change the recording device to ¡§Microphone (USB Device)¡¨, and playback devices to ¡§UAC Speaker¡¨.
		3.	User could use any recording software to test recording, and play audio to test playback. Or using listening to recording device to playback the audio from DMIC simultaneously.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    	(1) HIRC Auto Trim:
		1. The sample code uses HIRC auto trim function to stabilize the HIRC frequency.
    		2. TIMER0 handler will enable HIRC auto trim for every 10 second.