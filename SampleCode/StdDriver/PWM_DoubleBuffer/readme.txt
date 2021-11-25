/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	This sample code will use PWM0 channel 0 to output waveform using double buffer feature.
	(1)	Double buffers separates software writing and hardware action operation timing to prevent asynchronous operation problem due to software and hardware asynchronicity.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect to COM port to send out demo message (TX=PB8, RX=PB9).
	(2)	Connect PB.4(PWM0 CH0) to oscilloscope or similar devices to observer the waveform (It might require to connect the GND to devices).
	(3)	Compile and execute.
	(4)	Program test procedure ¡V
		1.	The program will first configure PWM channel 0 for demo.
		2.	User can observers the PWM output waveform from the oscilloscope.
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
