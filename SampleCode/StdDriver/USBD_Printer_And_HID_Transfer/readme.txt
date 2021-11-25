/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate how to implement a composite device.(USB micro printer device and HID Transfer).
    (1) HID Transfer -
        1. Transfer data between USB device and PC through USB HID interface.
		2. A windows tool is also included in this sample code to connect with a USB device.
    (2) Micro printer device -
	    1. Implement an USB printing support device.
		2. A window driver is also include in this sample code to setup before using. 
		
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Plug-in PC via USB. (VBUS=PB15, DN=PB13, DN=PB14)
    (2) Compiled to execute.
	(3) Micro printer part - Please refer '\Windows driver\Readme_58mm.doc' in the demo sample folder.
    (4) HID transfer part -
		1. Execute 'HIDTransferTest.exe' in the demo project folder on the PC. The windows test window will appear.
        2. Program test procedure -
          <1> The PC sends an EraseSectors command and the firmware will call 'HIDTrans_EraseSector' function in main.c, 
              then com port message will be 'Get erase secore request...'.
          <2> The PC sends a ReadPages command, the firmware will call 'HIDTrans_PrepareReadPage' function in main.c, 
              then the com port message will be 'Get read page request...'.
          <3> The PC sends a WritePages command, the firmware will call 'HIDTrans_PrepareWritePage' function in main.c, 
              then the com port message will be 'Get write page request...'; 
          <4> When a WritePages command finishes, the firmware will call 'HIDTrans_GetWriteData' function in main.c, 
              and the com port message will be 'Get write. Data finish message...'
          <5> Finally, the PC will send the ReadPages command again and compare it with the written data. 
              If it is the same, the test will be successful, but the test will fail;

---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    (1) PID is 0xAABB in this sample.(defined in printer_hidtrans.h)
    (2) Windows tool: 
        1. User need to input the specific PID for the USB HID device connected to PC.
        2. PID format with hexadecimal.
    (3) HIRC Auto Trim:
	1. The sample code uses HIRC auto trim function to stabilize the HIRC frequency.
    	2. TIMER0 handler will enable HIRC auto trim for every 10 second.