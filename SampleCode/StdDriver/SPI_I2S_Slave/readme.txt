/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate SPI data transfer.
    (1) Implement SPI transfer
    (2) Configure SPI1 as SPI_I2S Slave mode and demonstrate how SPI_I2S works in Slave mode.
    
		
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connects to comport to send out demo message (TX=PB8, RX=PB9)
    (2) Connect Pins: 
         PD5(Master) < - > PD5(Slave)
         PD4(Master) < - > PD4(Slave)
         PD3(Master) < - > PD2(Slave)
         PD2(Master) < - > PD3(Slave) 
    (3) Compiled to execute.
    (4) Program test procedure -
	1. set the source data and clear the destination buffer
 	2. Write to TX register
	3. Read received data 
	4. Check the received data
	5. print the current TX value and the new received value if received value changes    
      
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    (1) This sample code needs to work with SPI_I2S_Master.