/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	This sample code shows how to implement a HID device as mouse.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Compile and execute.
	(2)	Connect USB to USB port on the EVB (CN1). 
	(3)	Program test procedure ¡V
		1.	PC will recognize the device as mouse interface.
		2.	Device will control the mouse pointer doing a circular movement. At this point the demo is finished, and user can unplug the USB.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    	(1) HIRC Auto Trim:
		1. The sample code uses HIRC auto trim function to stabilize the HIRC frequency.
    		2. TIMER0 handler will enable HIRC auto trim for every 10 second.