/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	This sample code will use PWM0 channel 2 to capture the signal from PWM0 channel 0. 
	
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect to COM port to send out demo message (TX=PB8, RX=PB9).
	(2)	Connect PB.2(PWM0 CH2) to PB.0(PWM0 CH0).
	(3)	Compile and execute.
	(4)	Program test procedure ¡V
		1.	The program will first configure PWM channel 0 and 2 for demo.
		2.	When showing message ¡§Press any key to start PWM Capture Test¡¨, press any key to start demo.
		3.	Program will show the capture result and compare to the PWM output setting. If the result is matched, it will show ¡§Capture Test Pass!!¡¨.
		4.	After showing the test result, the demo is finished.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
