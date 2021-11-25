/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate I2S data transfer.
    (1) Implement how I2S works in Slave mode..
    (2) This sample code will transmit data the received from Master.

---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connects to comport to send out demo message (TX=PB8, RX=PB9)
    (2) Connect Pins: 
         Master PD6(BCLK) < - > Slave PD6(BCLK)
         Master PD5(DO) < - > Slave PD4(DI)
         Master PD4(DI) < - > Slave PD5(DO)
         Master PD3(LRCK) < - > Slave PD3(LRCK)
    (3) Compiled to execute.
    (4) Program test procedure -
		1. Execute the slave smaple code before master code starts I2S.
		2. The master will transmit data in "ai32OutBuf" to slave, and slave sample code will
		   return the received data from master.
		3. The Master sample code will show data received from slave.
      
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    (1) This sample code needs to work with I2S_Master.