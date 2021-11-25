/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	This sample code shows how to use WWDT Compare March Interrupt to reset system.
	(1)	WWDT Compare March Interrupt will trigger 10 times.
	(2)	After the 10th time interrupt -up, the system will be reset by WDT.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect to COM port to send out demo message (TX=PB8, RX=PB9).
	(2)	Compile and execute.
	(3)	Program test procedure ¡V
		1.	First, program will configure WWDT.
		2.	Next, the system will generate first WWDT compare match interrupt event after 932067.56 us.
		3.	Program will reload WWDT counter value to avoid WWDT time-out reset system occurred when interrupt counts less than 11.
		4.	Do not reload WWDT counter value to generate WWDT time-out reset system event when interrupt counts large than 10.
		5.	When WWDT interrupt counts reach 10, the system will reset and the program will show ¡§System has been reset by WWDT time-out reset event. ***¡¨, and the demo is finished.
---------------------------------------------------------------------------------------------------------
Note:
(1)	 User could use PA.0 to check WWDT compare match interrupt period time
