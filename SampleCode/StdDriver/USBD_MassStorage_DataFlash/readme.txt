/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate how to implement a USB Mass-Storage device.
    (1) Use embedded data flash as storage to implement a USB Mass-Storage device.
    (2) Data flash size is 64KB.(Definition in usbd_bot.h).
		
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Compiled to execute.
    (2) Plug-in PC via USB.(VBUS=PB15, DN=PB13, DN=PB14).
    (3) There will be a storage device for users to read/write data on the PC system windows.

---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    (1) PID is 0xB005 in this sample.(defined in masstorage.h)
    (2) HIRC Auto Trim:
	1. The sample code uses HIRC auto trim function to stabilize the HIRC frequency.
    	2. TIMER0 handler will enable HIRC auto trim for every 10 second.