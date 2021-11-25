/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate how to implement a composite device.(VCOM and HID Transfer)
    (1) HID Transfer -
        1. Transfer data between USB device and PC through USB HID interface.
		2. A windows tool is also included in this sample code to connect with a USB device.
    (2) VCOM -
	    1. Implement a USB virtual com port device.
		2. A window driver is also include in this sample code to setup before using. 
		
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Compile and download.
    (2) Connect USB to PC.
    (3) VCOM part - 
        1. Setup windows driver.(Driver in this sample code folder.)
        2. TX:PB8, RX:PB9
		3. Transfer data to other com port device via 3rd party tool.
    (4) HID Transfer part -
        1. Open windows tool on PC.(Tool in this sample code folder.)
        2. Execute 'HIDTransferTest.exe' to test transfer data.
		3. Source code of this window tool in the same folder.

---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    (1) PID is 0xDC00 in this sample.(defined in vcom_hidtrans.h)
    (2) Windows tool: 
        1. User need to input the specific PID for the USB HID device connected to PC.
        2. PID format with hexadecimal.
    (3) HIRC Auto Trim:
	1. The sample code uses HIRC auto trim function to stabilize the HIRC frequency.
    	2. TIMER0 handler will enable HIRC auto trim for every 10 second.