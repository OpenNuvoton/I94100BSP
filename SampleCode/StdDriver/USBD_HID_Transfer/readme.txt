/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate how to implement a humane interface device.
    (1) Transfer data from USB device to PC through USB HID interface.
    (2) Transfer data type is interrupt in/out.
		
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connects to comport to send out demo message (TX=PB8, RX=PB9)
    (2) Plug-in PC via USB. (VBUS=PB15, DN=PB13, DN=PB14)
    (3) Compiled to execute.
    (4) Execute 'HIDTransferTest.exe' in the demo project folder on the PC. The windows test window will appear.
    (5) Program test procedure -
        1. The PC sends an EraseSectors command and the firmware will call 'HIDTrans_EraseSector' function in main.c, 
           then com port message will be 'Get erase secore request...'.
        2. The PC sends a ReadPages command, the firmware will call 'HIDTrans_PrepareReadPage' function in main.c, 
           then the com port message will be 'Get read page request...'.
        3. The PC sends a WritePages command, the firmware will call 'HIDTrans_PrepareWritePage' function in main.c, 
           then the com port message will be 'Get write page request...'; 
        4. When a WritePages command finishes, the firmware will call 'HIDTrans_GetWriteData' function in main.c, 
           and the com port message will be 'Get write. Data finish message...'
        5. Finally, the PC will send the ReadPages command again and compare it with the written data. 
           If it is the same, the test will be successful, but the test will fail;

---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    (1) PID is 0x8230 in this sample.(defined in hid_trans.h)
    (2) Windows tool: 
        1. User need to input the specific PID for the USB HID device connected to PC.
        2. PID format with hexadecimal.
    (3) HIRC Auto Trim:
	1. The sample code uses HIRC auto trim function to stabilize the HIRC frequency.
    	2. TIMER0 handler will enable HIRC auto trim for every 10 second.