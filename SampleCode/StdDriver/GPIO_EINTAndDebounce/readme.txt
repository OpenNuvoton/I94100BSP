/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	Demonstrate how to use GPIO EINT and Debounce function.
	(1)	Using EINT0(PA.13, PA.15) and EINT1(PC.5) to demonstrate.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect to COM port to send out demo message (TX=PB8, RX=PB9).
	(2)	Connect pins PA.13, PA.15, PC.5 to external nodes which user can control like buttons or similar devices.
	(3)	Compile and execute.
	(4)	Program test procedure ¡V
		1.	Program configures PA.13 to be triggered by falling edge, PA.15 to be triggered by rising edge and PC.5 to be triggered by both edge.
		2.	When triggers each pin, the message will show "PC.5 EINT1 occurred.", "PA.13 EINT0 occurred." or "PA.15 EINT0 occurred.\n".
		3.	The demonstration is finished, and user could continue testing the EINT trigger.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
