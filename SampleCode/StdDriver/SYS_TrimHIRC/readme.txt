/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	This sample code shows how to trim HIRC by 32k X¡¦Tal and USB.
	(1)	Demo will trim 48MHZ and 49.152MHz by both external clock source.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect to COM port to send out demo message (TX=PB8, RX=PB9).\
	(2)	Plug 32K X¡¦Tal to PC.0(X32_OUT) and PC.1(X32_IN).
	(3)	Connect USB to USB port on the EVB (CN1). 
	(4)	Compile and execute.
	(5)	Program test procedure ¡V
		1.	User could choose which HIRC clock and source to trim from.
		2.	After the input of selected option, the program will start to trim HIRC clock.
		3.	When trimming is done, program will show the result and end the demonstration.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
