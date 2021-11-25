/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright(c) Nuvoton Technology Corp. All rights reserved.                                              */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/
---------------------------------------------------------------------------------------------------------
Purpose:
---------------------------------------------------------------------------------------------------------
    Demonstrate how to use SPI quad-mode to read/write data to external SPI-FLash.
    (1) Implement SPI quad-mode data transfer.
    (2) SPI0 will be configured as Master mode.
        SPI0(master) will send command via one-bit mode, and read write data through quad-mode.
	(3)	Using W25Q256FV serial flash memory in this demonstration.
		
---------------------------------------------------------------------------------------------------------
Operation:
---------------------------------------------------------------------------------------------------------
    (1) Connects to comport to send out demo message (TX=PB8, RX=PB9)
    (2) Connect Pins: 
		SPI0_SS  (PA.6) <--> SS
		SPI0_CLK (PA.5) <--> CLK
		SPI0_MISO0(PA.4) <--> DO
		SPI0_MOSI0(PA.3) <--> DI
		SPI0_MISO1(PA.2) <--> HOLD
		SPI0_MOSI1(PA.1) <--> WP
		VCC <--> W25Q256FV VCC
		GND <--> W25Q256FV Ground
    (3) Compiled to execute.
    (4) Program test procedure -
		1. press enter to start transmission
		2. Wait for the flash to stable to read ID and  status register.
		2. enter to SPIFlashDemo() function demonstration.
        3. result will show "SPI Flash erase/program/read sample end" if transmission is completed.
		
---------------------------------------------------------------------------------------------------------
Note:
---------------------------------------------------------------------------------------------------------
    