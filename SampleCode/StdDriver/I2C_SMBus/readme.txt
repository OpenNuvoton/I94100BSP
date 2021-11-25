/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate how to implement use I2C Serial Interface 
    (1) Implement how to control SMBus interface and use SMBus protocol between Host and Slave.
		
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connects to comport to send out demo message (TX=PB8, RX=PB9)
    (2) Connect Pins: 
	   PA.9  (Master) < -- > PC.0 (Slave)
	   PA.10 (Master) < -- > PC.1 (Slave)
	   PA.12 (Master) < -- > PC.2 (Slave)
	   PA.11 (Master) < -- > PC.3 (Slave)
    (3) The pull up resistance need to connect PA.10, PA.9 and 3.3V.
    (4) Compiled to execute.
    (5) Program test procedure -
          1. Press the corresponding option
              	[1] SMBus Send Bytes Protocol with PEC Test
		[2] SMBus Alert Function Test
		[3] Simple ARP and ACK Control by Manual Test
		[0] Exit                                     
         
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    (1) This sample need to pull up resistance.
  