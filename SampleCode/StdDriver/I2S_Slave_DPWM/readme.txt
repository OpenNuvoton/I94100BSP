/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate I2S audio data receive and using DPWM to play audio.
    (1) Implement how I2S works in Slave mode..
    (2) This sample code will transmit data the received from Master.
	(3)	BSP provides "USBD_UAC_I2S_Output" to work as I2S Master.

---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connects to comport to send out demo message (TX=PB8, RX=PB9)
    (2) Connect Pins: 
         PD6(Master) < - > PD6(Slave)
         PD5(Master) < - > PD4(Slave)
         PD4(Master) < - > PD5(Slave)
         PD3(Master) < - > PD3(Slave)
	(3)	Connect speaker to PC.10, PC.11 (DPWM_RN, RP) and PC.12 and PC.13 (DPWM_LN, LP).
    (4) Compiled to execute.
    (5) Program test procedure -
		1. The program will initiate and starts I2S and DPWM.
		2. When receiving audio data from master, the program will play audio by DPWM.
      
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    (1) This sample code needs to work with a I2S master device("USBD_UAC_I2S_Output").