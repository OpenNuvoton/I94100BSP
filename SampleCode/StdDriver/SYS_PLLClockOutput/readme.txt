/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	This sample code shows how to change PLL clock setting and enable clock output.
	(1)	Configure PA.13 as clock output.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect to COM port to send out demo message (TX=PB8, RX=PB9).
	(2)	Connect PA.13 to oscilloscope or similar devices to observer the waveform (It might require to connect the GND to devices).
	(3)	Compile and execute.
	(4)	Program test procedure ¡V
		1.	When showing ¡§Press any key to continue ...¡¨, input any key to start the demo.
		2.	The program will change the PLL clock multiple times, and user can observe the PLL clock output from PA.13.
		3.	After the PLL testing, the system will reset, and program will return to procedure ¡¥1.¡¦.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
