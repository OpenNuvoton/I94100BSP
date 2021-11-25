/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	Demonstrate how to use GPIO output and input mode.
(1)	Using PB.3(output) and PD.7(input) to demonstrate GPIO output and input mode.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
(1)	Connect to COM port to send out demo message (TX=PB8, RX=PB9).
(2)	Connect pins PB.3 to PD.7.
(3)	Compile and execute.
(4)	Program test procedure ¡V
1.	After showing ¡§Press any key to start test by using [Pin Data Input/Output Control]¡¨, press any key to continue.
2.	The program will set PB.3(output) to low, and then waiting for PD.7(input) value turned low.
3.	Next, the program will set PB.3(output) to high, and then waiting for PD.7(input) value turned high.
4.	If the PD.7(input) get the correct value within the limited time in both test, the program will show ¡§[OK]¡¨ and end the demo.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
