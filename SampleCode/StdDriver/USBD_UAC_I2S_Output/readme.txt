/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
	This sample code shows how to implement a USB playback device using I2S output. The I2S output should
	be received by a slave device.
	
	BSP provides "I2S_Slave_DPWM" as a slave device sample code to playback audio.
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
	(1)	Connect Pins: 
         PD6(Master) < - > PD6(Slave)
         PD5(Master) < - > PD4(Slave)
         PD4(Master) < - > PD5(Slave)
         PD3(Master) < - > PD3(Slave) 
	(2)	Compile and execute.
	(3)	Connect USB to USB port on the EVB (CN1). 
	(4)	Program test procedure ¡V
		1.	PC will recognize the device as USB playback device.
		2.	On PC, user should change the playback devices to ¡§UAC Speaker¡¨.
		3.	User could play audio to test playback.
		4.	I2S will send audio data to slave device(in this case is using "I2S_Slave_DPWM" sample).
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
   	(1) HIRC Auto Trim:
		1. The sample code uses HIRC auto trim function to stabilize the HIRC frequency.
    	2. TIMER0 handler will enable HIRC auto trim for every 10 second.
	(2) This sample code needs to work with a I2S slave device("I2S_Slave_DPWM").
	(3)	PDMA_ENABLE 
		1: using PDMA transfer data from buffer to I2S TX FIFO.
		0: using I2S interrupt to transfer data from buffer to I2S TX FIFO.