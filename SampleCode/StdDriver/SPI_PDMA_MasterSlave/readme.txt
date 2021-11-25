/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate how to implement use SPI  Interface 
    (1) Implement SPI data transfer with PDMA.
    (2) SPI0 will be configured as Master mode and SPI1 will be configured as Slave mode.
    (3) Both TX PDMA function and RX PDMA function will be enabled.
		
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
	2. configure SPI_master's RX&TX channel and SPI_slave's RX&TX channel. 
        3. result show "Exit SPI driver sample code." if transmission is done.
    
      
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
 
  