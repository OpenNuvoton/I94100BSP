/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate SPI data transfer.
    (1) Implement SPI data transfer.
    (2) SPI0 will be configured as Master mode and SPI1 will be configured as Slave mode.
        SPI0(master) transfer via interrupt and SPI1(slave) transfer via main loop.
		
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connects to comport to send out demo message (TX=PB8, RX=PB9)
    (2) Connect Pins: 
	 SPI0_SS  (PA.6) <--> SPI1_SS  (PD.5)
         SPI0_CLK (PA.5) <--> SPI1_CLK (PD.4)
         SPI0_MISO(PA.4) <--> SPI1_MISO(PD.3)
         SPI0_MOSI(PA.3) <--> SPI1_MOSI(PD.2)  
    (3) Compiled to execute.
    (4) Program test procedure -
	1. press any key to start transmission
	2. enter to SpiLoopTest() function for SPI configuretion.
        3. result show "SPI0/1 Loop test  .......... [PASS] Exit SPI driver sample code." if transmission was Pass.
    
      
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    