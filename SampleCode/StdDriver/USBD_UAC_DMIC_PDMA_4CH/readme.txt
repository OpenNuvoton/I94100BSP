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
	
	User can chose how many channels to be enabled by enabling one of the record device and disabling threee other device.
	Four devices could enable 1, 2, 3 and 4 channels.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect EVB-I94124ADI-NAU85L40B to JP4 on EVB-I94100.
	(2)	Connect speaker to DPWM_R on JP5 on EVB.
	(3)	Compile and execute.
	(4)	Connect USB to USB port on the EVB (CN1). 
	(5)	Program test procedure ¡V
		1.	PC will recognize the device as USB recording and also playback device.
		2.	On PC, user should change the recording device to ¡§Microphone (USB Device)¡¨.
		3.	User could use any recording software to test recording. Or using listening to recording device to playback the audio from DMIC simultaneously.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
