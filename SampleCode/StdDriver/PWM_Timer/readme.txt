/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	This sample code will use PWM0 channel 0 to work as timer.
	
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect to COM port to send out demo message (TX=PB8, RX=PB9).
	(3)	Compile and execute.
	(4)	Program test procedure ?V
		1.	The program will first configure PWM channel 0 frequency to 1Hz.
		2.	Then enable zero point interrupt to server as timer interrupt.
		2.	UART will print out how many second have passed.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
