/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	Demonstrate how to use GPIO INT function.
	(1)	Using PB.2 and PC.5 to demonstrate GPIO interrupt.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect to COM port to send out demo message (TX=PB8, RX=PB9).
	(2)	Connect pins PB.2 and PC.5 to external nodes which user can control like buttons or similar devices.
	(3)	Compile and execute.
	(4)	Program test procedure ¡V
		1.	Program configures PB.2 to be triggered by rising edge, and PC.5 to be triggered by falling edge.
		2.	When triggers each pin, the message will show "PB.2 INT occurred." or "PC.5 INT occurred.".
		3.	The demonstration is finished, and user could continue testing the GPIO interrupt.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
