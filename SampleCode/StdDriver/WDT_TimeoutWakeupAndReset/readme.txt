/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	This sample code shows how to use WDT time-out to wake up system and also reset the system.
	(1)	WDT time-out will wake up system from power down 10 times.
	(2)	After the 10th time wake-up, the system will be reset by WDT.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect to COM port to send out demo message (TX=PB8, RX=PB9).
	(2)	Compile and execute.
	(3)	Program test procedure ¡V
		1.	First, program will configure WDT.
		2.	Next, the system will enter power down mode and wait for WDT to wake up. 
		3.	After the wake-up, the system will enter power down mode again. This procedure will repeat 10 times.
		4.	When WDT interrupt counts reach 10, the system enter power down mode and wait for WDT to reset system.
		5.	When WDT successfully reset the system, the program will show ¡§*** System has been reset by WDT time-out event ***¡¨, and the demo is finished.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
	(1) User could use PA.0 to check WDT time-out interval