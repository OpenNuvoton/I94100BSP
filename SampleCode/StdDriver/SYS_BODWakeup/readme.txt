/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	This sample code shows how to configure ¡§Brown-Out detection¡¨ wakeup function.
	(1)	System will wake up from power down mode if brown-out event occurred.

---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect to COM port to send out demo message (TX=PB8, RX=PB9).
	(2)	Connect the EVB power¡¦s source to a variable voltage source, such as DC power supply,
		and set the voltage level at 3.3V.
	(3)	Compile and execute.
	(4)	Program test procedure ¡V
		1.	The program will first configure BOD level and interrupt function.
		2.	The program will enter power-down mode.
		3.	User could lower the voltage source of VCC to trigger BOD.
		4.	The BOD event will occurred and wake up the system.
		5.	When showing " System wake-up from Power-down mode.", the demo is finished. 
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
