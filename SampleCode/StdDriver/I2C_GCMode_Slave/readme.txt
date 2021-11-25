/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate how to implement use I2C Serial Interface 
    (1) Implementhow to receive data from Master in GC (General Call) mode.
		
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connects to comport to send out demo message (TX=PB8, RX=PB9)
    (2) Connect PA9(Master) < -- > PA9(Slave), PA10(Master)< -- > PA10(Slave)      
    (3) The pull up resistance need to connect PA.10, PA.9 and 3.3V.
    (4) Compiled to execute.
    (5) Program test procedure -
          1. Waiting master send message
	  2. when receive messag, show GC Mode receive data OK. 
	  
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    (1) This sample code needs to work with I2C_GCMode_Master
    (2) This sample need to pull up resistance.
    (3) Slave must running first.