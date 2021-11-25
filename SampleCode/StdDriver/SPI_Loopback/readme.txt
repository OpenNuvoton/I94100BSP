/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate SPI data transfer.
    (1) Implement SPI Master loop back transfer.
    (2) This sample code needs to connect SPI0_MISO0 pin and SPI0_MOSI0 pin together.
	It will compare the received data with transmitted data.
		
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connects to comport to send out demo message (TX=PB8, RX=PB9)
    (2) Connect Pins: 
         SPI0_MOSI(PA.3) <--> SPI0_MISO(PA.4)  
    (3) Compiled to execute.
    (4) Program test procedure -
	1. set the source data and clear the destination buffer
 	2. Write to TX register
	3. Read received data 
	4. Check the received data
	5. Result show PASS if transmission done.
    
      
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    