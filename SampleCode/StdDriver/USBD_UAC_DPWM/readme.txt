/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	This sample code shows how to implement a USB playback device using DPWM.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect speaker to PC.10, PC.11 (DPWM_RN, RP) and PC.12 and PC.13 (DPWM_LN, LP).
	(2)	Compile and execute.
	(3)	Connect USB to USB port on the EVB (CN1). 
	(4)	Program test procedure ¡V
		1.	PC will recognize the device as USB playback device.
		2.	On PC, user should change the playback devices to ¡§UAC Speaker¡¨.
		3.	User could play audio to test playback. 
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
   	(1) HIRC Auto Trim:
		1. The sample code uses HIRC auto trim function to stabilize the HIRC frequency.
    		2. TIMER0 handler will enable HIRC auto trim for every 10 second.