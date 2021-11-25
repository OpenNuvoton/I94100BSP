/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	This sample code will output PWM signals between channel 0~3 with two kinds of different frequency and duty, and enable dead zone function on channel 0 and 2 pairs.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect to COM port to send out demo message (TX=PB8, RX=PB9).
	(2)	Connect PB.0, PB.1 (PWM0 CH0 and CH1) and PB.2, PB.3 (PWM0 CH2 and CH3) pairs respectively to oscilloscope or similar devices to observer the waveform (It might require to connect the GND to devices).
	(3)	Compile and execute.
	(4)	Program test procedure ¡V
		1.	The program will first configure PWM channel 0 and 2 for demo.
		2.	User can observers the PWM output waveform from the oscilloscope.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
