/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate SPI data transfer.
    (1) Implement how SPI_I2S works in Master mode..
    (2)  This sample code will transmit a TX value 50000 times, and then change to the next TX value.
		
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connects to comport to send out demo message (TX=PB8, RX=PB9)
    (2) Connect Pins: 
         PD5(Master) < - > PD5(Slave)
         PD4(Master) < - > PD4(Slave)
         PD3(Master) < - > PD2(Slave)
         PD2(Master) < - > PD3(Slave)  
    (3) Run the slave demo first to wait for master transmission 
    (4) Compiled to execute.
    (5) Program test procedure -
	1. Press any key to start
	2.  If received value changes, print the current TX value and the new received value.
    
      
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    (1) This sample code needs to work with SPI_I2S_Slave.