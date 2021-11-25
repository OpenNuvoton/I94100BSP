/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	Demonstrate how to use GPIO interrupt in power-down wake-up source.
	(1)	Using PB.3 as an interrupt source to wake-up chip from power-down.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect to COM port to send out demo message (TX=PB8, RX=PB9).
	(2)	Connect pins PB.3 to an external node which user can control to alter PB.3 voltage level.
		The PB.3 should be low before entering power down mode.
	(3)	Compile and execute.
	(4)	Program test procedure ¡V
		1.	When showing ¡§Enter to Power-Down ......¡¨, the program is entering the power-down mode.
		2.	Change the voltage of PB.3 from low to high to trigger interrupt.
		3.	If the interrupt has been triggered, the chip will be waking up from power-down mode.
		4.	After waking up from power-down mode, the program will show ¡§PB.3 INT occurred.¡¨, ¡§System waken-up done.¡¨ and the demo is finished.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
